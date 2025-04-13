#ifndef SHARED_H
#define SHARED_H

#include <stdbool.h>
#include <semaphore.h>

// Estructura para cada jugador.
typedef struct {
    char name[16];
    unsigned int points;
    unsigned int invalidMovements;
    unsigned int validMovements;
    unsigned short x, y;
    pid_t pid;
    bool blocked;
} Player;

// Estructura del estado del juego.
typedef struct {
    unsigned short width;
    unsigned short height;
    unsigned int playerCount;
    Player players[9];
    bool gameOver;
    int board[];  // Tablero dinámico: fila-0, fila-1, ..., fila-(height-1)
} GameState;

// Estructura para la sincronización.
typedef struct {
    sem_t printNeeded; // Indica a la vista que hay cambios por imprimir
    sem_t printDone; // Indica al máster que la vista terminó de imprimir
    sem_t turnstile; // Mutex para evitar inanición del máster al acceder al estado
    sem_t resourceAccess; // Mutex para el estado del juego (escritor)
    sem_t readerCountMutex; // Mutex para la variable readerCount (lectores)
    unsigned int readerCount; // Cantidad de jugadores leyendo el estado
} GameSync;

#endif