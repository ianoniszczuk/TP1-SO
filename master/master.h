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
#include "../libraries/sharedHeaders.h"

void init_shared_memory(GameState **state,size_t board_size,unsigned short width,unsigned short height,int num_players,unsigned int seed);

void init_sync_struct(GameSync **sync);

void create_pipes();

void create_players_and_view(char *view_path, char *player_paths[],int num_players,int pipes[][2],GameState *state, char * arg_width, char* arg_height);

void distribute_players(GameState *state);

void handle_movements(GameState *state,GameSync *sync,int pipes[][2], int num_players, int timeout,int delay_m);

void clean_resources(GameState *state, size_t state_size, GameSync *sync);

void printFinalResults(GameState *state);