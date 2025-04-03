#include "view.h"

//Definiciones

int puntajes[9];

bool flag = false;

const char *colorMap[] = {
    ANSI_COLOR_WHITE,
    ANSI_BRIGHT_BLACK,
    ANSI_COLOR_BLUE,
    ANSI_COLOR_MAGENTA,
    ANSI_COLOR_CYAN,
    ANSI_COLOR_RED,
    ANSI_COLOR_GREEN,
    ANSI_COLOR_YELLOW,
    ANSI_COLOR_BLACK
};


int isHead(GameState * game, int value, int x, int y){
    return (x == game->players[-1*value].x && y == game->players[-1*value].y);
}

void printCellRow(int value, int py, int x, int y, GameState *game){
    for (int px = 0; px < CELL_SIZE; px++){
        if (value <= 0){
            printf("%s", colorMap[-1*value]);
            if (isHead(game, value, x, y)){
                if ((px == 0) && (py == CELL_SIZE/2)){
                    printf("█▓");
                }
                else if ((px == CELL_SIZE-1) && (py == CELL_SIZE/2)){
                    printf("▓█");
                }
                else{
                    printf("%s", game->players[-1*value].blocked ? "░░":"██");
                }
            }
            else {
                printf("██");
            }
        }
        else {
            if ((px == CELL_SIZE/2) && (py == px)){
                printf("%s%s %d", ANSI_BG_BLACK, ANSI_COLOR_BLACK, value);
            }
            else {
                printf("%s%s  ", ANSI_BG_BLACK, ANSI_COLOR_BLACK);
            }
        }
        printf(ANSI_COLOR_RESET);
    }
}

void printBoard(GameState *game) {

    const unsigned short width = game->width;
    const unsigned short height = game->height;
    const unsigned int player_count = game->player_count;

    printf("%s", ANSI_CLEAR_SCREEN);
    printf("%s", ANSI_CURSOR_HOME);
    
    for (int y = 0; y < height; y++) {
        for (int py = 0; py < CELL_SIZE; py++){
            printf(MARGIN_TAB);
            for (int x = 0; x < width; x++) {
                int toPrint = game->board[y*width + x];
                printCellRow(toPrint, py, x, y, game);
            }
            printf("\n");
        }
    }
    printf("\n");

    for(int i = 0; i < player_count; i++) {
        printf(ANSI_BOLD_WHITE);
        printf(MARGIN_TAB);
        printf("Player %s%d%s score: %d", colorMap[i], i, ANSI_BOLD_WHITE, game->players[i].points);
        printf(ANSI_COLOR_RESET);
        printf(" - ");
        printf("%d Invalid movements", game->players[i].invalid_movements);
        printf(ANSI_COLOR_RESET);
        if (game->players[i].blocked){
            printf(ANSI_COLOR_RED);
            printf(" - ");
            printf("Blocked!");
            printf(ANSI_COLOR_RESET);
        }
        printf("\n");
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
        printBoard(game);
        sem_post(&sync->printDone);  // Indica al máster que terminó de imprimir
    }

    //TODO: Chequera munmap 

    munmap(game, total_size);
    munmap(sync, sizeof(GameSync));
    return EXIT_SUCCESS;
}