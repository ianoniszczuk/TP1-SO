#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/select.h>
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

void init_shared_memory(GameState **state,size_t board_size,unsigned short width,unsigned short height,int num_players,unsigned int seed);

void init_sync_struct(GameSync **sync);

void create_pipes();

void create_players_and_view(char *view_path, char* players_paths[], int num_players, int pipes[][2],GameState *state);

void distribute_players(GameState *state);

void handle_movements(GameState *state,GameSync *sync,int pipes[][2], int num_players, int timeout,int delay_m);

void clean_resources(GameState *state, size_t state_size, GameSync *sync);
