#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/select.h>
#include <getopt.h>
#include <signal.h>
#include "../libraries/sharedHeaders.h"

#define MAX_PLAYERS 9

typedef struct Options {
    int width;
    int height;
    int delay_ms;
    int timeout_sec;
    unsigned int seed;
    char * view_path;
    int num_players;
    char * player_paths[MAX_PLAYERS];
} Options;

void init_shared_memory(GameState **state,size_t board_size, Options * options);

void init_sync_struct(GameSync **sync);

void create_pipes();

void create_players_and_view(char *view_path, char *player_paths[],int num_players,int pipes[][2],GameState *state, char * arg_width, char* arg_height);

void distribute_players(GameState *state);

void handle_movements(GameState *state,GameSync *sync,int pipes[][2], int num_players, int timeout,int delay_m);

void clean_resources(GameState *state, size_t state_size, GameSync *sync);

void print_options(Options * options);

void printFinalResults(GameState *state);

void clean_and_end();