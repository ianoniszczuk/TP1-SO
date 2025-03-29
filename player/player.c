#include "player.h"

int main(void) {

    int player_number;

    int dx[8] = {0,1,1,1,0,-1,-1,-1};
    int dy[8] = {-1,-1,0,1,1,1,0,-1};

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

    for(int i =0;i<game->player_count;i++){
        if(getpid() == game->players[i].pid){
            player_number = i;
            break;
        }
    }

    // Bucle principal del jugador: mientras no esté bloqueado, lee el estado y envía movimientos.

    while (!game->game_over) {

        //Chequeo si estoy bloqueado

        // if(game->players[player_number].blocked){
        //     break;
        // }
        
        sem_wait(&sync->C);
        sem_wait(&sync->E);
        sync->F++;
        if(sync->F == 1){
            sem_wait(&sync->D);
        }
        sem_post(&sync->E);
        sem_post(&sync->C);

        sem_wait(&sync->E);
        sync->F--;
        if(sync->F == 0){
            sem_post(&sync->D);
        }
        sem_post(&sync->E);

        //TODO : Logica de player, mejorar dsp para competencia
        unsigned char movimiento = rand() % 8;  // Movimiento aleatorio entre 0 y 7

        // Enviar el movimiento al máster a través del pipe (STDOUT se asume que está conectado al pipe del máster)
        if (write(STDOUT_FILENO, &movimiento, sizeof(movimiento)) < 0) {
            perror("Error escribiendo movimiento");
            break;
        }

        sleep(1);  // Retardo de 1 segundo para simular tiempo de decisión, admeas lo indico el profe
    }

    munmap(game, total_size);
    munmap(sync, sizeof(GameSync));
    return EXIT_SUCCESS;
}
