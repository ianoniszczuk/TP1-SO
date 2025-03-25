#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>
#include <semaphore.h>

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
    int board[];  // Tablero dinámico: fila-0, fila-1, ..., fila-(height-1)
} GameState;

// Estructura para la sincronización.
typedef struct {
    sem_t A; // Indica a la vista que hay cambios por imprimir
    sem_t B; // Indica al máster que la vista terminó de imprimir
    sem_t C; // Mutex para evitar inanición del máster al acceder al estado
    sem_t D; // Mutex para el estado del juego (escritor)
    sem_t E; // Mutex para la variable F (lectores)
    unsigned int F; // Cantidad de jugadores leyendo el estado
} GameSync;

// Función que imprime el tablero.
void print_board(GameState *game) {
    printf("Tablero (%hu x %hu):\n", game->width, game->height);
    for (int i = 0; i < game->height; i++) {
        for (int j = 0; j < game->width; j++) {
            printf("%d ", game->board[i * game->width + j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(void) {
    int fd_state = shm_open("/game_state", O_RDONLY, 0666);
    int fd_sync = shm_open("/game_sync", O_RDONLY, 0666);
    if (fd_state == -1 || fd_sync == -1) {
        perror("Error abriendo la memoria compartida");
        exit(EXIT_FAILURE);
    }

    // Mapeo inicial para obtener dimensiones.
    GameState *game = mmap(NULL, sizeof(GameState), PROT_READ, MAP_SHARED, fd_state, 0);
    if (game == MAP_FAILED) {
        perror("Error mapeando game_state");
        exit(EXIT_FAILURE);
    }
    unsigned short width = game->width, height = game->height;
    size_t total_size = sizeof(GameState) + width * height * sizeof(int);
    munmap(game, sizeof(GameState));  // Liberamos el mapeo inicial

    // Mapeamos el estado completo (estructura + tablero).
    game = mmap(NULL, total_size, PROT_READ, MAP_SHARED, fd_state, 0);
    if (game == MAP_FAILED) {
        perror("Error mapeando estado completo");
        exit(EXIT_FAILURE);
    }
    close(fd_state);

    // Mapeamos la memoria compartida para la sincronización.
    GameSync *sync = mmap(NULL, sizeof(GameSync), PROT_READ, MAP_SHARED, fd_sync, 0);
    if (sync == MAP_FAILED) {
        perror("Error mapeando game_sync");
        exit(EXIT_FAILURE);
    }
    close(fd_sync);

    // Bucle principal: espera la señal del máster para imprimir y notifica cuando termina.
    while (!game->game_over) {
        sem_wait(&sync->A);  // Espera a que el máster indique que hay cambios
        print_board(game);
        sem_post(&sync->B);  // Indica al máster que terminó de imprimir
    }

    munmap(game, total_size);
    munmap(sync, sizeof(GameSync));
    return EXIT_SUCCESS;
}
