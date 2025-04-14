#include "gameLogicAdt.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <sys/time.h>

GameLogicAdt initGameLogic(GameState *state, GameSync *sync, int timeout, int delayMs) {    
    GameLogicAdt logic;
    logic.state = state;
    logic.sync = sync;
    logic.timeout = timeout;
    logic.delayMs = delayMs;
    return logic;
}

void distributePlayers(GameLogicAdt *logic) {
    for (unsigned int i = 0; i < logic->state->playerCount; i++) {
        int x, y, cellIndex;
        do {
            x = rand() % logic->state->width;
            y = rand() % logic->state->height;
            cellIndex = y * logic->state->width + x;
        } while (logic->state->board[cellIndex] <= 0); 
        
        logic->state->players[i].x = x;
        logic->state->players[i].y = y;
        logic->state->players[i].points = 0;
        logic->state->players[i].invalidMovements = 0;
        logic->state->players[i].validMovements = 0;
        logic->state->players[i].blocked = false;
        
        logic->state->board[cellIndex] = -i;
    }
}

void runGameLoop(GameLogicAdt *logic, ProcessManagerAdt *pm) {
    fd_set rfds;
    int maxFd = 0;
    int currentPlayer = 0;
    time_t lastMoveAttemptTime = time(NULL);
    struct timeval tv;
    
    int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

    while (!logic->state->gameOver) {
        time_t currentTime = time(NULL);
        double timeSinceLastMove = difftime(currentTime, lastMoveAttemptTime);
        
        if (timeSinceLastMove > logic->timeout) {
            sem_wait(&logic->sync->turnstile);
            sem_wait(&logic->sync->resourceAccess);
            
            logic->state->gameOver = true;

            sem_post(&logic->sync->printNeeded);
            sem_wait(&logic->sync->printDone);

            printf("Timeout: No player movement detected for %d seconds\n", logic->timeout);

            sem_post(&logic->sync->resourceAccess);
            sem_post(&logic->sync->turnstile);
            
            break;
        }

        FD_ZERO(&rfds);

        for (int i = 0; i < pm->numPlayers; i++) {
            FD_SET(pm->pipes[i][0], &rfds);
            if (pm->pipes[i][0] > maxFd) {
                maxFd = pm->pipes[i][0];
            } 
        }

        tv.tv_sec = logic->timeout;
        tv.tv_usec = 0;

        int activity = select(maxFd + 1, &rfds, NULL, NULL, &tv);

        if (activity < 0) {
            perror("select");
            break;
        }
        
        lastMoveAttemptTime = currentTime;
        
        for (int i = 0; i < pm->numPlayers; i++) { 
            int idx = (currentPlayer + i) % pm->numPlayers;
            
            if (FD_ISSET(pm->pipes[idx][0], &rfds)) {
                sem_wait(&logic->sync->turnstile);
                sem_wait(&logic->sync->resourceAccess);

                int update = true;

                unsigned char move;
                int bytes = read(pm->pipes[idx][0], &move, sizeof(move));
                
                if (bytes <= 0) {
                    logic->state->players[idx].blocked = true;
                    update = false;
                }

                if (bytes > 0 && move > 7) {
                    // advoid malicious attempt of player
                    move = move % 8; 
                }

                int newX = logic->state->players[idx].x + dx[move];
                int newY = logic->state->players[idx].y + dy[move];
                int cellIndex = newY * logic->state->width + newX;

                if (newX < 0 || newX >= logic->state->width || newY < 0 || newY >= logic->state->height) {
                    logic->state->players[idx].invalidMovements++;
                    update = false;
                } else if (logic->state->board[cellIndex] <= 0) {
                    logic->state->players[idx].invalidMovements++;
                    update = false;
                }

                if (update) {
                    logic->state->players[idx].validMovements++;
                    logic->state->players[idx].points += logic->state->board[cellIndex];
                    logic->state->players[idx].x = newX;
                    logic->state->players[idx].y = newY;
                    logic->state->board[cellIndex] = -(idx);
                }

                for (int k = 0; k < pm->numPlayers; k++) {
                    bool wasBlocked = logic->state->players[k].blocked;
                    bool blockedFlag = true;

                    for (int j = 0; j < 8; j++) {
                        int toCheckX = logic->state->players[k].x + dx[j];
                        int toCheckY = logic->state->players[k].y + dy[j];

                        if (toCheckX >= 0 && toCheckY >= 0 && 
                            toCheckX < logic->state->width && 
                            toCheckY < logic->state->height && 
                            logic->state->board[toCheckY * logic->state->width + toCheckX] > 0) {
                            blockedFlag = false;
                        }
                    }

                    logic->state->players[k].blocked = blockedFlag;
                    
                    if (blockedFlag && !wasBlocked) {
                        printf("Player %d has been blocked\n", k);
                        closePlayerPipe(pm, k);
                    }
                }

                logic->state->gameOver = true;
                for (int i = 0; i < pm->numPlayers; i++) {
                    if (!logic->state->players[i].blocked) {
                        logic->state->gameOver = false;
                        break;
                    }
                }
            
                sem_post(&logic->sync->resourceAccess);
                sem_post(&logic->sync->turnstile);

                currentPlayer = (idx + 1) % pm->numPlayers;

                sem_post(&logic->sync->printNeeded);
                sem_wait(&logic->sync->printDone);  

                usleep(logic->delayMs * 1000);

                break;
            }
        }

        if (logic->state->gameOver) {
            sem_post(&logic->sync->printNeeded);
        }
    }
}

void printFinalResults(GameLogicAdt *logic, int returnCodes[]) {
    int playerCount = logic->state->playerCount;
    int indices[playerCount];
    
    for (int i = 0; i < playerCount; i++) {
        indices[i] = i;
    }
    
    for (int i = 0; i < playerCount - 1; i++) {
        for (int j = 0; j < playerCount - i - 1; j++) {
            int idx1 = indices[j];
            int idx2 = indices[j + 1];
            Player p1 = logic->state->players[idx1];
            Player p2 = logic->state->players[idx2];
            
            if (p1.points < p2.points) {
                int temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            } else if (p1.points == p2.points && p1.validMovements > p2.validMovements) {
                int temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            } else if (p1.points == p2.points && p1.validMovements == p2.validMovements && 
                     p1.invalidMovements > p2.invalidMovements) {
                int temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            }
        }
    }
    
    printf("\n===== RESULTADOS FINALES =====\n");
    
    int rank = 1;
    for (int i = 0; i < playerCount; i++) {
        int idx = indices[i];
        Player currentPlayer = logic->state->players[idx];
        
        if (i > 0) {
            int prevIdx = indices[i-1];
            Player prevPlayer = logic->state->players[prevIdx];
            
            if (currentPlayer.points != prevPlayer.points || 
                currentPlayer.validMovements != prevPlayer.validMovements || 
                currentPlayer.invalidMovements != prevPlayer.invalidMovements) {
                rank = i + 1;
            }
        }
        
        printf("Rank %d: Jugador %d, puntos: %u, movimientos válidos: %u, movimientos inválidos: %u\n",
               rank, idx, currentPlayer.points, currentPlayer.validMovements, 
               currentPlayer.invalidMovements);
    }
    
    printf("===========================\n");
} 