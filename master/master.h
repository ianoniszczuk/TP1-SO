#ifndef MASTER_H
#define MASTER_H

#include "gameConfig.h"
#include "sharedHeaders.h"
#include "gameLogicAdt.h"
#include <stdio.h>      // For printf, fprintf, stderr
#include <stdlib.h>     // For exit, EXIT_FAILURE, srand, rand
#include <stddef.h>     // For NULL
#include <signal.h>     // For signal, SIGINT
#include <unistd.h>     // For getopt, optarg, optind, sleep, usleep, fork, execve, close
#include <fcntl.h>      // For O_RDWR, O_CREAT
#include <sys/mman.h>   // For mmap, munmap, PROT_READ, PROT_WRITE, MAP_SHARED, MAP_FAILED
#include <sys/types.h>  // For pid_t
#include <sys/wait.h>   // For waitpid, WIFEXITED, WEXITSTATUS
#include <time.h>       // For time, difftime

void print_options(Options *options);
void init_shared_memory(GameState **state, size_t board_size, Options *options);
void create_pipes(int pipes[][2], int num_players);
void create_players_and_view(char *view_path, char *player_paths[], int num_players, int pipes[][2], GameState *state, char *arg_width, char *arg_height);
void distribute_players(GameState *state);
void handle_movements(GameState *state, GameSync *sync, int pipes[][2], int num_players, int timeout, int delay_ms);
void printFinalResults(GameLogicAdt *logic, int returnCodes[]);

#endif