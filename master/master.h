#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>
#include <semaphore.h>
#include <time.h>
#include <getopt.h>

// Estructura para cada jugador.
typedef struct {
    char name[16];
    unsigned int points;
    unsigned int invalid_movements;
    unsigned int valid_movements;
    unsigned short x, y;
    pid_t pid;
    bool blocked;
} Player;

// Estructura del estado del juego.
typedef struct {
    unsigned short width;
    unsigned short height;
    unsigned int player_count;
    Player players[9];
    bool game_over;
    int board[];  // Tablero dinámico
} GameState;

// Estructura para la sincronización.
typedef struct {
    sem_t A; // Vista
    sem_t B; // Master
    sem_t C; // Mutex para evitar inanición del máster al acceder al estado
    sem_t D; // Mutex para el estado del juego (escritor)
    sem_t E; // Mutex para la variable F (lectores)
    unsigned int F; // Cantidad de jugadores leyendo el estado
} GameSync;

void init_shared_memory();

void init_sync_struct();

void create_pipes();

void create_players();

void create_view();

void run_game();