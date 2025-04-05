#include "master.h"

#define MAX_PLAYERS 9

#define GAME_STATE "/game_state"
#define GAME_SYNC "/game_sync"

int processReturn[9];

int main(int argc, char * argv[]){

    unsigned short width = 10;   // Valor por defecto y mínimo: 10
    unsigned short height = 10;  // Valor por defecto y mínimo: 10
    int delay_ms = 200;          // Valor por defecto: 200 ms
    int timeout_sec = 10;        // Valor por defecto: 10 seg
    unsigned int seed = 0;       // Si no se pasa, se usará time(NULL)
    char *view_path = NULL;      // Por defecto: sin vista
    char * arg_width = NULL;
    char * arg_height = NULL;

    // Array para guardar las rutas de los jugadores.
    char *player_paths[MAX_PLAYERS];
    int num_players = 0;
    int players_flag = 0;        // Bandera para saber si se encontró la opción -p

    int opt;
    // "w:h:d:t:s:v:p" indica:
    // - w,h,d,t,s,v toman argumento (después de cada opción, se espera un valor)
    // - p es una bandera que indica que a continuación vendrán los jugadores.
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
                fprintf(stderr,
                        "Uso: %s [-w width] [-h height] [-d delay] [-t timeout] [-s seed] [-v view] -p player1 [player2 ... player9]\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Verificar que se haya indicado la opción -p para jugadores.
    if (!players_flag) {
        fprintf(stderr, "Error: Falta la opción -p para especificar los jugadores.\n");
        exit(EXIT_FAILURE);
    }

    // Todos los argumentos restantes (desde optind) se consideran rutas de jugadores.
    while (optind < argc && num_players < MAX_PLAYERS) {
        player_paths[num_players++] = argv[optind++];
    }

    // Validar que se haya proporcionado al menos 1 jugador.
    if (num_players < 1) {
        fprintf(stderr, "Error: Se debe proporcionar al menos 1 jugador.\n");
        exit(EXIT_FAILURE);
    }

    // Si no se proporcionó seed, usar time(NULL) como valor por defecto.
    if (seed == 0) {
        seed = (unsigned int)time(NULL);
    }

    // Inicializar la memoria compartida para el estado del juego.

    GameState *state;
    GameSync *sync; 

    size_t board_size = width * height * sizeof(int);

    init_shared_memory(&state, board_size, width, height, num_players, seed);

    init_sync_struct(&sync);

    int pipes[num_players][2];

    create_pipes(pipes, num_players); //Crear todos los pipes para los players

    create_players_and_view(view_path, player_paths ,num_players,pipes,state, arg_width, arg_height); //Crear los procesos de los players 

    distribute_players(state);

    handle_movements(state,sync,pipes, num_players,timeout_sec,delay_ms); //Bucle principal

    int status;

    //while(wait(&status) > 0);

    for(int i = 0;i<num_players;i++){
        pid_t pid = waitpid(state->players[i].pid, &status, 0);
        
        if(pid > 0){
            if(WIFEXITED(status)){
                processReturn[i] = WEXITSTATUS(status);
            }
        }
        else processReturn[i] = -1;            
    } //Espero a que terminen los hijos

    printFinalResults(state);

    clean_resources(state, sizeof(GameState) + board_size, sync); 

    return 0;
}    

void printFinalResults(GameState *state){
    for(int i = 0;i<state->player_count;i++){
        printf("Jugador %d (%d), puntos: %d, movimientos invalidos: %d\n",i,processReturn[i],state->players[i].points,state->players[i].invalid_movements);
    }
}

void init_shared_memory(GameState **state, size_t board_size, unsigned short width, unsigned short height, int num_players, unsigned int seed){

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

    if((*state) == MAP_FAILED){
        perror("Error mapeando la memoria compartida");
        exit(EXIT_FAILURE);
    }
    
    (*state)->width = width;
    (*state)->height = height;
    (*state)->player_count = num_players;
    (*state)->game_over = false;

    srand(seed);

    for(int i = 0;i<width * height;i++){
        (*state)->board[i] = (rand() % 9) + 1;
    }

    close(fd);   
}

void init_sync_struct(GameSync **sync){
    int fd = shm_open(GAME_SYNC, O_CREAT | O_RDWR, 0666);
    if(fd == -1){
        perror("shm_open GAME_SYNC");
        exit(EXIT_FAILURE);
    }

    if(ftruncate(fd, sizeof(GameSync)) == -1){
        perror("ftruncate GAME_SYNC");
        exit(EXIT_FAILURE);
    }

    *sync = mmap(NULL, sizeof(GameSync), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(*sync == MAP_FAILED){
        perror("Error mapeando game_sync");
        exit(EXIT_FAILURE);
    }

    // Semaforos para la sincronizacion de la view y el master

    sem_init(&((*sync)->printNeeded), 1, 0);
    sem_init(&((*sync)->printDone), 1, 0);
    // Semáforos para la sincronización máster/players (C, D y E) inician en 1.

    sem_init(&((*sync)->C), 1, 1);
    sem_init(&((*sync)->D), 1, 1);
    sem_init(&((*sync)->E), 1, 1);
    (*sync)->F = 0;
    close(fd);

}

void create_pipes(int pipes[][2], int num_players){
    for(int i = 0;i<num_players;i++){
        if(pipe(pipes[i]) == -1){
            perror("Error creando los pipes");
            exit(EXIT_FAILURE);
        }
    }
}

void create_players_and_view(char *view_path, char *player_paths[],int num_players,int pipes[][2],GameState *state, char * arg_width, char* arg_height){

    pid_t pid;

    char * args[2] = {arg_width,arg_height};


    if(view_path != NULL){
        pid = fork();
        if(pid == -1){
            perror("Error en fork");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0){

            
            execve(view_path, args,NULL); //esto es sin argumentos, hay q crearlos
            perror("Error en execve");
            exit(EXIT_FAILURE);
        }
    }

    for(int i = 0; i<num_players;i++){
        pid = fork();
        if(pid == -1){
            perror("Error en fork");
            exit(EXIT_FAILURE);
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

            execve(player_paths[i], args,NULL); //esto es sin argumentos, hay q crearlos
            perror("Error en execve");
            exit(EXIT_FAILURE);
        } else {

            close(pipes[i][1]);
            state->players[i].pid = pid;
        }
    }
    
}

void distribute_players(GameState *state){

    for(int i = 0;i<state->player_count;i++){
        state->players[i].x = i;
        state->players[i].y = 0;
        state->players[i].points = 0;
        state->players[i].invalid_movements =0;
        state->players[i].valid_movements =0;
        state->players[i].blocked =false;
        state->board[state->players[i].y * state->width+ state->players[i].x] = -i;
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

            sem_wait(&sync->C);
            sem_wait(&sync->D);
            
            state->game_over = true;

            sem_post(&sync->printNeeded);
            sem_wait(&sync->printDone);

            printf("Timeout\n");

            sem_post(&sync->D);
            sem_post(&sync->C);
            
           
            
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

                sem_wait(&sync->C);
                sem_wait(&sync->D);

                int update = true;

                unsigned char move;
                int bytes = read(pipes[idx][0],&move,sizeof(move));

                if(bytes <= 0){
                    state->players[idx].blocked = true;
                    update = false;
                }

                int new_x = state->players[idx].x + dx[move];
                int new_y = state->players[idx].y + dy[move];
                int cell_index = new_y * state->width + new_x;

                //Todas las condiciones para las cuales hago un movimiento invalido

                if(new_x < 0 || new_x >= state->width || new_y < 0 || new_y >= state->height || move > 7 || state->board[cell_index] <= 0){
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
            
                sem_post(&sync->D);
                sem_post(&sync->C);

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

void clean_resources(GameState *state, size_t state_size, GameSync *sync) {
    // Destruir semáforos
    sem_destroy(&sync->printNeeded);
    sem_destroy(&sync->printDone);
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