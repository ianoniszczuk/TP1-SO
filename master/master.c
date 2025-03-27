#include "master.h"

#define GAME_STATE "/game_state"
#define GAME_SYNC "/game_sync"
#define MAX_PLAYERS 9

int main(int argc, char* argv[]){

    unsigned short width = 10;
    unsigned short height = 10;
    int delay_ms = 200;
    unsigned int timeout_sec = 10;
    unsigned int seed = time(NULL);
    char *view_path = NULL;
    char *players_paths[9] = {NULL};
    int num_players = 0;

    int opt;

    while ((opt = getopt(argc, argv, "w:h:d:t:s:v:")) != -1) {
        switch (opt) {
            case 'w':
                width = (unsigned short)atoi(optarg);
                if (width < 10) width = 10;
                break;
            case 'h':
                height = (unsigned short)atoi(optarg);
                if (height < 10) height = 10;
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
            default:
                fprintf(stderr, "Uso: %s [-w width] [-h height] [-d delay_ms] [-t timeout_sec] [-s seed] [-v view_path] player1 player2 ... player9\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Falta la opción -p con las rutas de los jugadores\n");
        exit(EXIT_FAILURE);
    }
    // Se espera la opción "-p"
    if (strcmp(argv[optind], "-p") != 0) {
        fprintf(stderr, "Se esperaba -p para indicar los jugadores\n");
        exit(EXIT_FAILURE);
    }
    optind++; // Saltar la opción -p
    // Cargar las rutas de jugadores
    while (optind < argc && num_players < 9) {
        players_paths[num_players++] = argv[optind++];
    }
    if (num_players < 1) {
        fprintf(stderr, "Debe especificarse al menos un jugador\n");
        exit(EXIT_FAILURE);
    }

    GameState *state;
    GameSync *sync;
    size_t board_size = width * height * sizeof(int);

    init_shared_memory(&state, board_size, width, height, num_players, seed);

    init_sync_struct(&sync);

    int pipes[num_players][2];

    create_pipes(pipes); //Crear todos los pipes para los players

    create_players_and_view(view_path, players_paths,num_players,pipes); //Crear los procesos de los players con fork

    distribute_players(state);

    handle_movements(state,sync,pipes, num_players,timeout_sec); //Bucle principal

    int status;

    while(wait(&status) > 0); //Espero a q terminen los hijos

    //wait_for_children();//Espero a q terminen los hijos.

    clean_resources(state, sizeof(GameState) + board_size, sync);

    printf("Juego terminado\n");

    return 0;
}

void create_pipes(int pipes[][2]){
    for(unsigned int i = 0; i<MAX_PLAYERS;i++){
        if(pipe(pipes[i]) == -1){
            perror("Error creando el pipe");
            exit(EXIT_FAILURE);
        }
    }
}

void init_shared_memory(GameState **state,size_t board_size,unsigned short width,unsigned short height,int num_players,unsigned int seed){
    int fd = shm_open(GAME_STATE, O_RDWR | O_CREAT, 0666);
    if(fd==-1){
        perror("Error creando la memoria compartida");
        exit(EXIT_FAILURE);
    }
    size_t total_size = sizeof(GameState) + board_size;
    if(ftruncate(fd, total_size) == -1){
        perror("Error al truncar la memoria compartida");
        exit(EXIT_FAILURE);
    }
    *state = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(*state == MAP_FAILED){
        perror("Error mapeando la memoria compartida");
        exit(EXIT_FAILURE);
    }

    (*state)->width = width;
    (*state)->height = height;
    (*state)->player_count = num_players;
    (*state)->game_over = false;

    srand(seed);

    for(unsigned int i = 0;i<width * height;i++){
        (*state)->board[i] = (rand() % 10);
    }

    close(fd);
}

void init_sync_struct(GameSync **sync){

    int fd = shm_open(GAME_SYNC, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open GAME_SYNC");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(fd, sizeof(GameSync)) == -1) {
        perror("ftruncate GAME_SYNC");
        exit(EXIT_FAILURE);
    }
    *sync = mmap(NULL, sizeof(GameSync), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (*sync == MAP_FAILED) {
        perror("mmap GAME_SYNC");
        exit(EXIT_FAILURE);
    }

    sem_init(&((*sync)->A), 1, 0);
    sem_init(&((*sync)->B), 1, 0);
    // Semáforos para la sincronización máster/players (C, D y E) inician en 1.
    sem_init(&((*sync)->C), 1, 1);
    sem_init(&((*sync)->D), 1, 1);
    sem_init(&((*sync)->E), 1, 1);
    (*sync)->F = 0;
    close(fd);

}

void create_players_and_view(char *view_path, char* players_paths[], int num_players, int pipes[][2]){
    pid_t pid;
    if(view_path != NULL){
        pid = fork();
        if(pid == -1){
            perror("Error en fork");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0){
            execl(view_path, view_path, NULL); //esto es sin argumentos, hay q crearlos
            perror("Error en execl");
            exit(EXIT_FAILURE);
        }
    }
    //Creo procesos de jugador
    for(unsigned int i = 0; i<num_players;i++){
        pid = fork();
        if(pid == -1){
            perror("Error en fork");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0){

            if(dup2(pipes[i][2], STDOUT_FILENO) == -1){
                perror("Error en dup2");
                exit(EXIT_FAILURE);
            }

            //Cierro todo menos el pipe de escritura del proceso en el q estoy

            for(int j = 0; j<num_players;j++){
                close(pipes[j][0]);
                if(i != j){
                    close(pipes[j][1]);
                }
            }

            execl(players_paths[i], players_paths[i], NULL); //esto es sin argumentos, hay q crearlos
            perror("Error en execl");
            exit(EXIT_FAILURE);
        }

    }

    //Cierro los pipes de lectura de los players en el proceso padre

    for(int i = 0;i<num_players;i++){
        close(pipes[i][1]);
    }


}

void distribute_players(GameState *state){
    for(unsigned int i = 0; i<state->player_count;i++){
        state->players[i].x = i % state->width;
        state->players[i].y = 0;
        state->players[i].points = 0;
        state->players[i].invalid_movements = 0;
        state->players[i].valid_movements = 0;
        state->players[i].pid = 0; // Se actualizará luego si se requiere
        state->players[i].blocked = false;

        // La celda inicial no debe dar recompensa
        state->board[state->players[i].y * state->width + state->players[i].x] = 0;
    }
}

void handle_movements(GameState *state,GameSync *sync,int pipes[][2], int num_players, int timeout){ //Bucle principal q maneja basicamente todo
    
    int max_fd = 0;
    fd_set rfds;
    struct timeval tv;
    time_t last_valid_time = time(NULL);
    int current_player = 0;

    int dx[8] = {0,1,1,1,0,-1,-1,-1};
    int dy[8] = {-1,-1,0,1,1,1,0,-1};

    while(!state->game_over){

        FD_ZERO(&rfds);

        for(int i = 0; i<num_players;i++){
            FD_SET(pipes[i][0],&rfds);
            if(pipes[i][0] > max_fd){
                max_fd = pipes[i][0];
            }
        }

        tv.tv_sec = 1;
        tv.tv_sec = 0;
        int activity = select(max_fd + 1, &rfds, NULL, NULL, &tv);

        if(activity < 0){
            perror("select");
            break;
        }

        if(activity == 0){
            if(difftime(time(NULL),last_valid_time) > timeout){
                printf("Timeout\n");
                state->game_over = true;
                break;
            }
            continue;
        }

        for(int i = 0;i<num_players;i++){
            int idx = (current_player + i) % num_players;
            if(FD_ISSET(pipes[idx][0],&rfds)){

                unsigned char move;
                int bytes = read(pipes[i][0],&move,sizeof(move));
                if(bytes <= 0){
                    state->players[i].blocked = true;
                    continue;
                }
                if(move > 7){
                    state->players[i].invalid_movements++;
                    continue;
                }

                int new_x = state->players[i].x + dx[move];
                int new_y = state->players[i].y + dy[move];

                if(new_x < 0 || new_x >= state->width || new_y < 0 || new_y >= state->height){
                    state->players[i].invalid_movements++;
                    continue;
                }

                int cell_index = new_y * state->width + new_x;

                if(cell_index <= 0){
                    state->players[i].invalid_movements++;
                    continue;
                }

                sem_wait(&sync->C);
                sem_wait(&sync->D);

                state->players[i].valid_movements++;
                state->players[i].points += state->board[cell_index];
                state->players[i].x = state->players[i].x + dx[move];
                state->players[i].y = state->players[i].y + dy[move];
                state->board[cell_index] = -(idx);


                sem_post(&sync->D);
                sem_post(&sync->C);

                last_valid_time = time(NULL);
                current_player = (idx + 1) % num_players;

                sem_post(&sync->A); //Indico a la vista q hay cambios
                sem_wait(&sync->B); //Espero a q la vista termine de imprimir

                sleep(1);
                break;
            }
        }
    }
}

void clean_resources(GameState *state, size_t state_size, GameSync *sync) {
    // Destruir semáforos
    sem_destroy(&sync->A);
    sem_destroy(&sync->B);
    sem_destroy(&sync->C);
    sem_destroy(&sync->D);
    sem_destroy(&sync->E);
    // Desmapear memorias compartidas
    munmap(state, state_size);
    munmap(sync, sizeof(GameSync));
    // Eliminar objetos de memoria compartida
    shm_unlink(GAME_STATE);
    shm_unlink(GAME_SYNC);
}

