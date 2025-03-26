#include "view.h"

//Definiciones

int puntajes[9];

bool flag = false;

// Función que imprime el tablero.

void print_board(GameState *game, int *matrix) {

    const unsigned short width = game->width;
    const unsigned short height = game->height;
    const unsigned int player_count = game->player_count;

    printf("Tablero (%hu x %hu):\n", width, height);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            for(int k = 0; k < player_count; k++) {
                flag = false;
                if(game->players[k].x == j && game->players[k].y == i) {
                    if(matrix[i * width + j] > 0) puntajes[k] += matrix[i * width + j];
                    matrix[i * width + j] = -k;
                    continue;
                }
            }
            if(matrix[i * width + j] == 0){
                printf("-0 ");
            }
            else if(matrix[i * width + j] < 0){
                printf("%d ",matrix[i * width + j]);
            } else printf(" %d ", matrix[i * width + j]);
        }
        printf("\n");
    }
    printf("\n");

    for(int i = 0; i < player_count; i++) {
        printf("Jugador numero %d, posicion actual : (%d;%d), puntaje : %d\n", i, game->players[i].x, game->players[i].y, puntajes[i]);
    }

    printf("\n");
}

int main(void) {
    
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
    // Bucle principal: espera la señal del máster para imprimir y notifica cuando termina.

    int matrix[width * height];

    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            matrix[i * width + j] = game->board[i * width + j];
        }
    }

    while (!game->game_over) {
        sleep(2);
        sem_wait(&sync->A);  // Espera a que el máster indique que hay cambios
        print_board(game, matrix);
        sem_post(&sync->B);  // Indica al máster que terminó de imprimir
    }

    munmap(game, total_size);
    munmap(sync, sizeof(GameSync));
    return EXIT_SUCCESS;
}