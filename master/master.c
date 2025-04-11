#include "master.h"
#include "gameStateAdt.h"
#include "gameSyncAdt.h"
#include "gameConfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/stat.h>  // For fchmod
#include <string.h>    // For strdup

#define GAME_STATE "/game_state"
#define GAME_SYNC "/game_sync"

int processReturn[MAX_PLAYERS];

GameState *g_state = NULL;
size_t g_state_size = 0;
GameSync *g_sync = NULL;

void clean_and_exit(GameStateAdt *gameStateAdt, GameSyncAdt *gameSyncAdt) {
    if (gameStateAdt && gameSyncAdt) {
        cleanupGameState(gameStateAdt);
        cleanupGameSync(gameSyncAdt);
    }
    exit(EXIT_FAILURE);
}

void exit_signal(int sig) {
    clean_and_exit(NULL, NULL);
}

void create_pipes(int pipes[][2], int num_players);
void create_players_and_view(char *view_path, char *player_paths[], int num_players, int pipes[][2], GameState *state, char *arg_width, char *arg_height);
void distribute_players(GameState *state);
void handle_movements(GameState *state, GameSync *sync, int pipes[][2], int num_players, int timeout, int delay_ms);
void printFinalResults(GameState *state);

int main(int argc, char *argv[]) {
    signal(SIGINT, exit_signal);

    unsigned short width = 10;
    unsigned short height = 10;
    int delay_ms = 200;
    int timeout_sec = 10;
    unsigned int seed = 0;
    char *view_path = NULL;
    char *arg_width = NULL;
    char *arg_height = NULL;

    char *player_paths[MAX_PLAYERS];
    int num_players = 0;
    int players_flag = 0;

    int opt;
    while ((opt = getopt(argc, argv, "w:h:d:t:s:v:p")) != -1) {
        switch (opt) {
            case 'w':
                arg_width = optarg;
                width = (unsigned short)atoi(optarg);
                if (width < 10) {
                    fprintf(stderr, "Advertencia: width menor a 10, se ajusta a 10.\n");
                    width = 10;
                }
                break;
            case 'h':
                arg_height = optarg;
                height = (unsigned short)atoi(optarg);
                if (height < 10) {
                    fprintf(stderr, "Advertencia: height menor a 10, se ajusta a 10.\n");
                    height = 10;
                }
                break;
            case 'd':
                delay_ms = atoi(optarg);
                break;
            case 't':
                timeout_sec = atoi(optarg);
                break;
            case 's':
                seed = (unsigned int)atoi(optarg);
                break;
            case 'v':
                view_path = optarg;
                break;
            case 'p':
                players_flag = 1;
                break;
            case '?':
            default:
                fprintf(stderr, "Uso: %s [-w width] [-h height] [-d delay] [-t timeout] [-s seed] [-v view] -p player1 [player2 ... player9]\n", argv[0]);
                clean_and_exit(NULL, NULL);
        }
    }

    if (!players_flag) {
        fprintf(stderr, "Error: Falta la opción -p para especificar los jugadores.\n");
        clean_and_exit(NULL, NULL);
    }

    while (optind < argc && num_players < MAX_PLAYERS) {
        player_paths[num_players++] = argv[optind++];
    }

    if (num_players < 1) {
        fprintf(stderr, "Error: Se debe proporcionar al menos 1 jugador.\n");
        clean_and_exit(NULL, NULL);
    }

    if (seed == 0) {
        seed = (unsigned int)time(NULL);
    }

    Options options = {
        .width = width,
        .height = height,
        .delay_ms = delay_ms,
        .timeout_sec = timeout_sec,
        .seed = seed,
        .view_path = view_path,
        .num_players = num_players,
    };

    for (int i = 0; i < num_players; i++) {
        options.player_paths[i] = player_paths[i];
    }

    if (arg_height == NULL) {
        arg_height = malloc(20);
        if (!arg_height) {
            perror("Error allocating memory for arg_height");
            clean_and_exit(NULL, NULL);
        }
        sprintf(arg_height, "%d", height);
    }

    if (arg_width == NULL) {
        arg_width = malloc(20);
        if (!arg_width) {
            perror("Error allocating memory for arg_width");
            free(arg_height);  // Free arg_height if allocated
            clean_and_exit(NULL, NULL);
        }
        sprintf(arg_width, "%d", width);
    }

    size_t board_size = width * height * sizeof(int);
    GameStateAdt gameStateAdt = initGameState(board_size, &options);
    GameSyncAdt gameSyncAdt = initGameSync();

    g_state = gameStateAdt.state;
    g_state_size = gameStateAdt.size;
    g_sync = gameSyncAdt.sync;

    int pipes[num_players][2];
    create_pipes(pipes, num_players);
    create_players_and_view(view_path, player_paths, num_players, pipes, gameStateAdt.state, arg_width, arg_height);
    
    // Free allocated memory for arguments
    if (arg_width != optarg && arg_width != NULL) {
        free(arg_width);
    }
    if (arg_height != optarg && arg_height != NULL) {
        free(arg_height);
    }

    distribute_players(gameStateAdt.state);
    print_options(&options);
    sleep(1);
    handle_movements(gameStateAdt.state, gameSyncAdt.sync, pipes, num_players, timeout_sec, delay_ms);

    int status;
    for (int i = 0; i < num_players; i++) {
        pid_t pid = waitpid(gameStateAdt.state->players[i].pid, &status, 0);
        if (pid > 0) {
            if (WIFEXITED(status)) {
                processReturn[i] = WEXITSTATUS(status);
            }
        } else {
            processReturn[i] = -1;
        }
    }

    printFinalResults(gameStateAdt.state);
    cleanupGameState(&gameStateAdt);
    cleanupGameSync(&gameSyncAdt);

    return 0;
}

void print_options(Options *options) {
    printf("width: %d\n", options->width);
    printf("height: %d\n", options->height);
    printf("delay: %d\n", options->delay_ms);
    printf("timeout: %d\n", options->timeout_sec);
    printf("seed: %d\n", options->seed);
    printf("view: %s\n", options->view_path);
    printf("num_players: %d\n", options->num_players);
    for (int i = 0; i < options->num_players; i++) {
        printf("\t%s\n", options->player_paths[i]);
    }
}

void printFinalResults(GameState *state) {
    for (unsigned int i = 0; i < state->player_count; i++) {
        printf("Jugador %d (%d), puntos: %d, movimientos invalidos: %d\n", i, processReturn[i], state->players[i].points, state->players[i].invalid_movements);
    }
}

void init_shared_memory(GameState **state, size_t board_size, Options * options){

    int fd = shm_open(GAME_STATE, O_RDWR | O_CREAT, 0666);
    if(fd==-1){
        perror("Error creando la memoria compartida");
        clean_and_exit(NULL, NULL);
    }

    size_t total_size = sizeof(GameState) + board_size;
    
    if(ftruncate(fd, total_size) == -1){
        perror("Error al truncar la memoria compartida");
        clean_and_exit(NULL, NULL);
    }

    *state = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if((*state) == MAP_FAILED){
        perror("Error mapeando la memoria compartida");
        clean_and_exit(NULL, NULL);
    }
    
    (*state)->width = options->width;
    (*state)->height = options->height;
    (*state)->player_count = options->num_players;;
    (*state)->game_over = false;

    srand(options->seed);

    for(int i = 0;i<options->width * options->height;i++){
        (*state)->board[i] = (rand() % 9) + 1;
    }

    if (fchmod(fd, 0444) == -1) {
        perror("fchmod");
        clean_and_exit(NULL, NULL);
    }    

    close(fd);   
}

void init_sync_struct(GameSync **sync){
    int fd = shm_open(GAME_SYNC, O_CREAT | O_RDWR, 0666);
    if(fd == -1){
        perror("shm_open GAME_SYNC");
        clean_and_exit(NULL, NULL);
    }

    if(ftruncate(fd, sizeof(GameSync)) == -1){
        perror("ftruncate GAME_SYNC");
        clean_and_exit(NULL, NULL);
    }

    *sync = mmap(NULL, sizeof(GameSync), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(*sync == MAP_FAILED){
        perror("Error mapeando game_sync");
        clean_and_exit(NULL, NULL);
    }

    // Semaforos para la sincronizacion de la view y el master

    sem_init(&((*sync)->printNeeded), 1, 0);
    sem_init(&((*sync)->printDone), 1, 0);
    // Semáforos para la sincronización máster/players (C, D y E) inician en 1.

    sem_init(&((*sync)->turnstile), 1, 1);
    sem_init(&((*sync)->resourceAccess), 1, 1);
    sem_init(&((*sync)->readerCountMutex), 1, 1);
    (*sync)->readerCount = 0;
    close(fd);

}

void create_pipes(int pipes[][2], int num_players){
    for(int i = 0;i<num_players;i++){
        if(pipe(pipes[i]) == -1){
            perror("Error creando los pipes");
            clean_and_exit(NULL, NULL);
        }
    }
}

void create_players_and_view(char *view_path, char *player_paths[],int num_players,int pipes[][2],GameState *state, char * arg_width, char* arg_height){

    pid_t pid;
    
    // Create dynamically allocated copies of arguments - argv[0] should be program name
    char **args = malloc(4 * sizeof(char *));
    if (!args) {
        perror("Error allocating memory for args");
        clean_and_exit(NULL, NULL);
    }
    
    // For the view
    if(view_path != NULL) {
        args[0] = strdup(view_path);  // Program name
        args[1] = strdup(arg_width);  // First arg (width)
        args[2] = strdup(arg_height); // Second arg (height)
        args[3] = NULL;               // NULL terminator
        
        if (!args[0] || !args[1] || !args[2]) {
            perror("Error duplicating view argument strings");
            for (int i = 0; i < 3; i++)
                if (args[i]) free(args[i]);
            free(args);
            clean_and_exit(NULL, NULL);
        }

        pid = fork();
        if(pid == -1){
            perror("Error en fork");
            for (int i = 0; i < 3; i++)
                if (args[i]) free(args[i]);
            free(args);
            clean_and_exit(NULL, NULL);
        }
        else if(pid == 0){
            // Child process will call execve, so we don't need to free in this branch
            execve(view_path, args, NULL);
            perror("Error en execve (view)");
            exit(EXIT_FAILURE); // Using exit directly as we're in the child
        }
        
        // Free view args in parent
        for (int i = 0; i < 3; i++)
            if (args[i]) free(args[i]);
    }

    for(int i = 0; i<num_players;i++){
        // Reallocate args for each player process
        args[0] = strdup(player_paths[i]); // Program name
        args[1] = strdup(arg_width);       // First arg (width)
        args[2] = strdup(arg_height);      // Second arg (height)
        args[3] = NULL;                    // NULL terminator
        
        if (!args[0] || !args[1] || !args[2]) {
            perror("Error duplicating player argument strings");
            for (int i = 0; i < 3; i++)
                if (args[i]) free(args[i]);
            free(args);
            clean_and_exit(NULL, NULL);
        }

        pid = fork();
        if(pid == -1){
            perror("Error en fork");
            for (int i = 0; i < 3; i++)
                if (args[i]) free(args[i]);
            free(args);
            clean_and_exit(NULL, NULL);
        }
        else if(pid == 0){
            //Cierro todos los pipes en el hijo menos el suyo de escritura 
            for(int j =0; j<num_players;j++){
                close(pipes[j][0]);
                if(i != j){
                    close(pipes[j][1]);
                }
            }

            if(dup2(pipes[i][1], STDOUT_FILENO) == -1){
                perror("Error en dup2");
                exit(EXIT_FAILURE);
            }

            execve(player_paths[i], args, NULL);
            perror("Error en execve (player)");
            exit(EXIT_FAILURE); // Using exit directly as we're in the child
        } else {
            close(pipes[i][1]);
            state->players[i].pid = pid;
            
            // Free player args in parent after fork
            for (int j = 0; j < 3; j++)
                if (args[j]) free(args[j]);
        }
    }
    
    // Free args array
    free(args);
}

void distribute_players(GameState *state){
    for(unsigned int i = 0; i < state->player_count; i++) {
        // Generate random positions until an unoccupied position is found
        int x, y, cell_index;
        do {
            x = rand() % state->width;
            y = rand() % state->height;
            cell_index = y * state->width + x;
        } while (state->board[cell_index] <= 0); // Check if cell is already occupied
        
        state->players[i].x = x;
        state->players[i].y = y;
        state->players[i].points = 0;
        state->players[i].invalid_movements = 0;
        state->players[i].valid_movements = 0;
        state->players[i].blocked = false;
        
        // Award initial points based on the cell value
        state->players[i].points = state->board[cell_index];
        
        // Mark the cell as occupied by this player
        state->board[cell_index] = -i;
    }
}

void handle_movements(GameState *state,GameSync *sync,int pipes[][2], int num_players, int timeout, int delay_ms){ 

    struct timeval tv;
    fd_set rfds;
    int max_fd = 0;
    int last_valid_time = time(NULL);
    int current_player = 0;
    
    int dx[8] = {0,1,1,1,0,-1,-1,-1};
    int dy[8] = {-1,-1,0,1,1,1,0,-1};

    while(!state->game_over){
        bool blocked_flag = true;        

        if(difftime(time(NULL),last_valid_time) > timeout){

            sem_wait(&sync->turnstile);
            sem_wait(&sync->resourceAccess);
            
            state->game_over = true;

            sem_post(&sync->printNeeded);
            sem_wait(&sync->printDone);

            printf("Timeout\n");

            sem_post(&sync->resourceAccess);
            sem_post(&sync->turnstile);
            
           
            
            break;
        }

        FD_ZERO(&rfds);

        for(int i=0; i<num_players;i++){
            FD_SET(pipes[i][0],&rfds);
            if(pipes[i][0] > max_fd){
                max_fd = pipes[i][0];
            } 
        }

        tv.tv_sec =1;
        tv.tv_usec =0;

        int activity = select(max_fd+1, &rfds, NULL, NULL, &tv);

        if(activity < 0){
            perror("select");
            break;
        }

        if(activity == 0){
            continue;
        }
        

        for(int i = 0;i<num_players;i++){ 
            int idx = (current_player + i) % num_players;
            
            if(FD_ISSET(pipes[idx][0],&rfds)){
                sem_wait(&sync->turnstile);
                sem_wait(&sync->resourceAccess);

                int update = true;

                unsigned char move;
                int bytes = read(pipes[idx][0],&move,sizeof(move));
                
                if(bytes <= 0){
                    state->players[idx].blocked = true;
                    update = false;
                }

                // Ensure move is within valid range
                if (bytes > 0 && move > 7) {
                    move = move % 8;  // Clamp to valid range 0-7
                }

                int new_x = state->players[idx].x + dx[move];
                int new_y = state->players[idx].y + dy[move];
                int cell_index = new_y * state->width + new_x;

                //Todas las condiciones para las cuales hago un movimiento invalido
                if(new_x < 0 || new_x >= state->width || new_y < 0 || new_y >= state->height) {
                    state->players[idx].invalid_movements++;
                    update = false;
                } else if(state->board[cell_index] <= 0) {
                    state->players[idx].invalid_movements++;
                    update = false;
                }

                if(update){
                    last_valid_time = time(NULL);
                    state->players[idx].valid_movements++;
                    state->players[idx].points += state->board[cell_index];
                    state->players[idx].x = state->players[idx].x + dx[move];
                    state->players[idx].y = state->players[idx].y + dy[move];
                    state->board[cell_index] = -(idx);
                
                }

                for(int k =0;k<num_players;k++){

                    blocked_flag = true;

                    for(int j = 0; j<8;j++){

                    int to_check_x =  state->players[k].x+ dx[j];
                    int to_check_y = state->players[k].y + dy[j];

                        if(to_check_x >= 0 && to_check_y >= 0 && to_check_x < state->width && to_check_y < state->height && state->board[to_check_y * state->width + to_check_x] > 0){
                            blocked_flag = false;
                        }
                    }

                    state->players[k].blocked = blocked_flag;
                }

                state->game_over = true;

                for(int i =0;i<num_players;i++){
                    if(!state->players[i].blocked){
                        state->game_over = false;
                    }
                }
            
                sem_post(&sync->resourceAccess);
                sem_post(&sync->turnstile);

                current_player = (idx + 1) % num_players;

                sem_post(&sync->printNeeded); //Indico a la vista q hay cambios
                sem_wait(&sync->printDone); //Espero a q la vista termine de imprimir

                usleep(delay_ms * 1000);

                break;
            }

        }

        if(state->game_over == true){
            sem_post(&sync->printNeeded);
        }
    }
}

