
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>
#include <semaphore.h>

typedef struct {
    char name[16]; // Nombre del jugador
    unsigned int points; // Puntaje
    unsigned int invaid_movements; // Cantidad de solicitudes de movimientos inválidas realizadas
    unsigned int valid_movements; // Cantidad de solicitudes de movimientos válidas realizadas
    unsigned short x, y; // Coordenadas x e y en el tablero
    pid_t pid; // Identificador de proceso
    bool blocked; // Indica si el jugador está bloqueado
} Player;

typedef struct {
    unsigned short width; // Ancho del tablero
    unsigned short height; // Alto del tablero
    unsigned int player_count; // Cantidad de jugadores
    Player players[9]; // Lista de jugadores
    bool game_over; // Indica si el juego se ha terminado
    int board[]; // Puntero al comienzo del tablero. fila-0, fila-1, ..., fila-n-1
} GameState;

typedef struct {
sem_t A; // Se usa para indicarle a la vista que hay cambios por imprimir
sem_t B; // Se usa para indicarle al master que la vista terminó de imprimir
sem_t C; // Mutex para evitar inanición del master al acceder al estado
sem_t D; // Mutex para el estado del juego
sem_t E; // Mutex para la siguiente variable
unsigned int F; // Cantidad de jugadores leyendo el estado
} GameSync;

void print_board(GameState *game){
    printf("Tablero\n de %d x %d\n",game->width,game->height);
    for(int i = 0; i<game->height;i++){
        for(int j = 0; j<game->width;j++){
            printf("%d ",game->board[i*game->width+j]);
        }
        printf("\n");
    }
}

void print_players(GameState *game){
    return;
}

int main(){

    int fd1 = shm_open("/game_state", O_RDONLY, 0666);
    int fd2 = shm_open("/game_sync", O_RDONLY, 0666);

    if(fd1 == -1 || fd2 == -1){
        perror("Error aca shm_error");
        return EXIT_FAILURE;
    }

    GameState *game = mmap(NULL, sizeof(GameState),PROT_READ, MAP_SHARED, fd1,0);
    GameSync *sync = mmap(NULL, sizeof(GameSync),PROT_READ, MAP_SHARED, fd2,0);

    if(game == MAP_FAILED || sync == MAP_FAILED){
        perror("Error en mmap1");
        close(fd1);
        close(fd2);
        exit(EXIT_FAILURE);
    }
    unsigned short width = game->width;
    unsigned short height = game->height;

    size_t total_size = sizeof(GameState) + width * height * sizeof(int);

    munmap(game,sizeof(GameState));

    game = mmap(NULL,total_size,PROT_READ, MAP_SHARED, fd1,0);

    if(game == MAP_FAILED){
        perror("Error en mmap2");
        close(fd1);
        close(fd2);
        exit(EXIT_FAILURE);
    }

    close(fd1);

    // while(!game->game_over){
    //     sem_wait(&sync->A);
    //     sem_post(&sync->B);
    // }

    print_board(game);


    munmap(game,total_size);
    munmap(sync,sizeof(GameSync));
    close(fd2);
    return EXIT_SUCCESS;

}