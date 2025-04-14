#ifndef SHARED_H
#define SHARED_H

#include <stdbool.h>
#include <semaphore.h>

typedef struct {
    char name[16];
    unsigned int points;
    unsigned int invalidMovements;
    unsigned int validMovements;
    unsigned short x, y;
    pid_t pid;
    bool blocked;
} Player;

typedef struct {
    unsigned short width;
    unsigned short height;
    unsigned int playerCount;
    Player players[9];
    bool gameOver;
    int board[]; 
} GameState;

typedef struct {
    sem_t printNeeded;
    sem_t printDone; 
    sem_t turnstile;
    sem_t resourceAccess; 
    sem_t readerCountMutex; 
    unsigned int readerCount; 
} GameSync;

#endif