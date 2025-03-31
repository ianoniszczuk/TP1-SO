#include "view.h"

//Definiciones

int puntajes[9];

bool flag = false;

// Función que imprime el tablero.

void print_board(GameState *game) {

    const unsigned short width = game->width;
    const unsigned short height = game->height;
    const unsigned int player_count = game->player_count;

    printf("Tablero (%hux %hu):\n", width, height);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int toPrint = game->board[y*width + x];
            printf("%s%d ", (toPrint < 0 ? "" : " "), toPrint);
        }
        printf("\n");
    }
    printf("\n");

    for(int i = 0; i < player_count; i++) {
        printf("Jugador  %d, posicion actual : (%d;%d), puntaje : %d, movs invalidos : %d, bloqueado? %d: \n", i, game->players[i].x, game->players[i].y, game->players[i].points,game->players[i].invalid_movements, game->players[i].blocked);
    }
    printf("\n");
}

int main(void) {

    // TODO: pasar a un archivo shm.c 
    // TODO : chequear tema 666
    
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


    //TODO : ver de sacar sleep(2)
    
    while (!game->game_over) {
        sem_wait(&sync->printNeeded);  // Espera a que el máster indique que hay cambios
        print_board(game);
        sem_post(&sync->printDone);  // Indica al máster que terminó de imprimir
    }

    //TODO: Chequera munmap 

    munmap(game, total_size);
    munmap(sync, sizeof(GameSync));
    return EXIT_SUCCESS;
}