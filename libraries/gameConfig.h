#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#define MAX_PLAYERS 9

typedef struct {
    unsigned short width;
    unsigned short height;
    int delay_ms;
    int timeout_sec;
    unsigned int seed;
    char *view_path;
    int num_players;
    char *player_paths[MAX_PLAYERS];
} Options;

#endif
