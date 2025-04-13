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
        // Generate random positions until an unoccupied position is found
        int x, y, cellIndex;
        do {
            x = rand() % logic->state->width;
            y = rand() % logic->state->height;
            cellIndex = y * logic->state->width + x;
            
            // Check if cell is already occupied by a player or has an invalid value
            // logic->state->board[cellIndex] <= 0 means it's already occupied by a player
        } while (logic->state->board[cellIndex] <= 0); // Check if cell is already occupied
        
        logic->state->players[i].x = x;
        logic->state->players[i].y = y;
        logic->state->players[i].points = 0; // Initial points set to 0, initial cell doesn't count
        logic->state->players[i].invalidMovements = 0;
        logic->state->players[i].validMovements = 0;
        logic->state->players[i].blocked = false;
        
        // Mark the cell as occupied by this player without adding its value to the score
        logic->state->board[cellIndex] = -i;
    }
    
    // Ensure all players start with zero points (initial cell doesn't count)
    for (unsigned int i = 0; i < logic->state->playerCount; i++) {
        logic->state->players[i].points = 0;
    }
}

void runGameLoop(GameLogicAdt *logic, ProcessManagerAdt *pm) {
    struct timeval tv;
    fd_set rfds;
    int maxFd = 0;
    int currentPlayer = 0;
    time_t lastMoveAttemptTime = time(NULL);  // Time of last move attempt
    
    int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

    while (!logic->state->gameOver) {
        // Check if game is taking too long (handles player infinite loops)
        time_t currentTime = time(NULL);
        double timeSinceLastMove = difftime(currentTime, lastMoveAttemptTime);
        
        // If we've received activity but no valid moves for too long, trigger timeout
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

        // Use a shorter timeout for select to check for timeout condition more frequently
        tv.tv_sec = 0;
        tv.tv_usec = 500000; // 500ms

        int activity = select(maxFd + 1, &rfds, NULL, NULL, &tv);

        if (activity < 0) {
            perror("select");
            break;
        }

        if (activity == 0) {
            // No activity detected, continue to check timeout
            continue;
        }
        
        // Update the last attempt time whenever we detect activity
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

                // Ensure move is within valid range
                if (bytes > 0 && move > 7) {
                    move = move % 8;  // Clamp to valid range 0-7
                }

                int newX = logic->state->players[idx].x + dx[move];
                int newY = logic->state->players[idx].y + dy[move];
                int cellIndex = newY * logic->state->width + newX;

                // Check for invalid movement conditions
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

                // Check if any players are blocked
                for (int k = 0; k < pm->numPlayers; k++) {
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
                }

                // Check if game is over (all players blocked)
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

                sem_post(&logic->sync->printNeeded); // Signal view to update display
                sem_wait(&logic->sync->printDone);   // Wait for view to finish

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
    for (unsigned int i = 0; i < logic->state->playerCount; i++) {
        printf("Jugador %d (%d), puntos: %d, movimientos invalidos: %d\n", 
               i, returnCodes[i], logic->state->players[i].points, 
               logic->state->players[i].invalidMovements);
    }
} 