#include "master.h"

#define MAX_PLAYERS 9

#define GAME_STATE "/game_state"
#define GAME_SYNC "/game_sync"

int main(int argc, char * argv[]){

    unsigned short width = 10;   // Valor por defecto y mínimo: 10
    unsigned short height = 10;  // Valor por defecto y mínimo: 10
    int delay_ms = 200;          // Valor por defecto: 200 ms
    int timeout_sec = 10;        // Valor por defecto: 10 seg
    unsigned int seed = 0;       // Si no se pasa, se usará time(NULL)
    char *view_path = NULL;      // Por defecto: sin vista

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
                width = (unsigned short)atoi(optarg);
                if (width < 10) {
                    fprintf(stderr, "Advertencia: width menor a 10, se ajusta a 10.\n");
                    width = 10;
                }
                break;
            case 'h':
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

    /* init_sync_struct(&sync);

    int pipes[num_players][2];

    create_pipes(pipes); //Crear todos los pipes para los players

    create_players_and_view(view_path, players_paths,num_players,pipes); //Crear los procesos de los players 

    distribute_players(state);

    handle_movements(state,sync,pipes, num_players,timeout_sec); //Bucle principal

    int status;

    while(wait(&status) > 0); //Espero a q terminen los hijos

    wait_for_children();//Espero a q terminen los hijos.

    clean_resources(state, sizeof(GameState) + board_size, sync); 

    printf("Juego terminado\n"); */

    return 0;
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
        (*state)->board[i] = (rand() % 10);
    }

    close(fd);   
}

