#include "argParserAdt.h"
#include "errorHandling.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

ArgParserAdt parseArguments(int argc, char *argv[], Options *options) {
    ArgParserAdt parser = {
        .argWidth = NULL,
        .argHeight = NULL
    };

    options->width = 10;
    options->height = 10;
    options->delayMs = 200;
    options->timeoutSec = 10;
    options->seed = 0;
    options->viewPath = NULL;
    options->numPlayers = 0;

    bool playersFlag = false;
    
    int opt;
    while ((opt = getopt(argc, argv, "w:h:d:t:s:v:p")) != -1) {
        switch (opt) {
            case 'w':
                parser.argWidth = optarg;
                options->width = (unsigned short)atoi(optarg);
                if (options->width < 10) {
                    ERROR_EXIT("Error: Minimal board dimensions: 10x10");
                }
                break;
            case 'h':
                parser.argHeight = optarg;
                options->height = (unsigned short)atoi(optarg);
                if (options->height < 10) {
                     ERROR_EXIT("Error: Minimal board dimensions: 10x10");
                }
                break;
            case 'd':
                options->delayMs = atoi(optarg);
                break;
            case 't':
                options->timeoutSec = atoi(optarg);
                break;
            case 's':
                options->seed = (unsigned int)atoi(optarg);
                break;
            case 'v':
                options->viewPath = optarg;
                break;
            case 'p':
                playersFlag = true;
                break;
            case '?':
            default:
                ERROR_EXIT("Invalid command line options");
        }
    }

    if (!playersFlag) {
        ERROR_EXIT("-p missing to specify players");
    }

    while (optind < argc && options->numPlayers < MAX_PLAYERS) {
        options->playerPaths[options->numPlayers++] = argv[optind++];
    }

    if (options->numPlayers < 1) {
        ERROR_EXIT("At least 1 player is needed");
    }
    if (options->numPlayers > 9) {
        ERROR_EXIT("At most 9 players are allowed");
    }

    if (options->delayMs > options->timeoutSec * 1000) {
        ERROR_EXIT("delay cannot be greater than timeout!");
    }

    if (options->seed == 0) {
        options->seed = (unsigned int)time(NULL);
    }

    if (parser.argWidth == NULL) {
        static char widthStr[MAX_DIMENSIONS_LEN];
        sprintf(widthStr, "%d", options->width);
        parser.argWidth = widthStr;
    }

    if (parser.argHeight == NULL) {
        static char heightStr[MAX_DIMENSIONS_LEN];
        sprintf(heightStr, "%d", options->height);
        parser.argHeight = heightStr;
    }

    return parser;
}