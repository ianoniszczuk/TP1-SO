#ifndef MASTER_H
#define MASTER_H

#include "gameConfig.h"
#include "sharedHeaders.h"
#include "gameLogicAdt.h"
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

void print_options(Options *options);
void init_shared_memory(GameState **state, size_t board_size, Options *options);
void create_pipes(int pipes[][2], int num_players);
void create_players_and_view(char *view_path, char *player_paths[], int num_players, int pipes[][2], GameState *state, char *arg_width, char *arg_height);
void distribute_players(GameState *state);
void handle_movements(GameState *state, GameSync *sync, int pipes[][2], int num_players, int timeout, int delay_ms);
void printFinalResults(GameLogicAdt *logic, int returnCodes[]);

#endif