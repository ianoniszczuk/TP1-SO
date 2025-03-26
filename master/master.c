#include "master.h"


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

    init_shared_memory();

    init_sync_struct();

    create_pipes(); //Crear todos los pipes para los players

    create_players_and_view(); //Crear los procesos de los players con fork

    distribute_players(); //Distribuir los players en el tablero

    handle_movements();

    int status;

    while(wait(&status) > 0);

    clean_resources();

    run_game(); //Aca van los ultimos 5 puntos las responsabilidades del master

}

void init_shared_memory(){



}