#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>
#include <semaphore.h>
#include <time.h>

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

int main(void) {
    srand(time(NULL));

    int fd_state = shm_open("/game_state", O_RDONLY, 0666);
    int fd_sync = shm_open("/game_sync", O_RDWR, 0666);
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
    GameSync *sync = mmap(NULL, sizeof(GameSync), PROT_READ | PROT_WRITE, MAP_SHARED, fd_sync, 0);
    if (sync == MAP_FAILED) {
        perror("Error mapeando game_sync");
        exit(EXIT_FAILURE);
    }
    close(fd_sync);

    // Bucle principal del jugador: mientras no esté bloqueado, lee el estado y envía movimientos.
    while (!game->game_over) {
        // Patrón lectores-escritores para leer el estado del juego.
        sem_wait(&sync->E);
        sync->F++;
        if (sync->F == 1) {
            sem_wait(&sync->D);  // El primer lector bloquea al escritor
        }
        sem_post(&sync->E);

        // Aquí se puede agregar lógica para elegir un movimiento válido; en este ejemplo se hace de forma aleatoria.
        unsigned char movimiento = rand() % 8;  // Movimiento aleatorio entre 0 y 7

        // Liberar la sección crítica de lectura.
        sem_wait(&sync->E);
        sync->F--;
        if (sync->F == 0) {
            sem_post(&sync->D);  // El último lector libera al escritor
        }
        sem_post(&sync->E);

        // Enviar el movimiento al máster a través del pipe (STDOUT se asume que está conectado al pipe del máster)
        if (write(STDOUT_FILENO, &movimiento, sizeof(movimiento)) < 0) {
            perror("Error escribiendo movimiento");
            break;
        }

        usleep(500000);  // Retardo de 0.5 segundos para simular tiempo de decisión
    }

    munmap(game, total_size);
    munmap(sync, sizeof(GameSync));
    return EXIT_SUCCESS;
}
