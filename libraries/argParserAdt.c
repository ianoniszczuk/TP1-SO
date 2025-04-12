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
        .argHeight = NULL,
        .dynamicWidth = false,
        .dynamicHeight = false
    };

    // Default values
    options->width = 10;
    options->height = 10;
    options->delay_ms = 200;
    options->timeout_sec = 10;
    options->seed = 0;
    options->view_path = NULL;
    options->num_players = 0;

    bool playersFlag = false;
    
    // Parse command line arguments
    int opt;
    while ((opt = getopt(argc, argv, "w:h:d:t:s:v:p")) != -1) {
        switch (opt) {
            case 'w':
                parser.argWidth = optarg;
                options->width = (unsigned short)atoi(optarg);
                if (options->width < 10) {
                    fprintf(stderr, "Warning: width less than 10, adjusting to 10\n");
                    options->width = 10;
                }
                break;
            case 'h':
                parser.argHeight = optarg;
                options->height = (unsigned short)atoi(optarg);
                if (options->height < 10) {
                    fprintf(stderr, "Warning: height less than 10, adjusting to 10\n");
                    options->height = 10;
                }
                break;
            case 'd':
                options->delay_ms = atoi(optarg);
                break;
            case 't':
                options->timeout_sec = atoi(optarg);
                break;
            case 's':
                options->seed = (unsigned int)atoi(optarg);
                break;
            case 'v':
                options->view_path = optarg;
                break;
            case 'p':
                playersFlag = true;
                break;
            case '?':
            default:
                perror("Invalid command line options");
                exit(EXIT_FAILURE);
        }
    }

    // Validate players
    if (!playersFlag) {
        perror("Falta la opción -p para especificar los jugadores");
        exit(EXIT_FAILURE);
    }

    // Parse player paths
    while (optind < argc && options->num_players < MAX_PLAYERS) {
        options->player_paths[options->num_players++] = argv[optind++];
    }

    if (options->num_players < 1) {
        perror("Se debe proporcionar al menos 1 jugador");
        exit(EXIT_FAILURE);
    }

    // Set seed if not specified
    if (options->seed == 0) {
        options->seed = (unsigned int)time(NULL);
    }

    // Create dynamic width/height strings if not provided
    if (parser.argHeight == NULL) {
        parser.argHeight = malloc(20);
        if (!parser.argHeight) {
            perror("Error allocating memory for argHeight");
            exit(EXIT_FAILURE);
        }
        sprintf(parser.argHeight, "%d", options->height);
        parser.dynamicHeight = true;
    }

    if (parser.argWidth == NULL) {
        parser.argWidth = malloc(20);
        if (!parser.argWidth) {
            perror("Error allocating memory for argWidth");
            if (parser.dynamicHeight) {
                free(parser.argHeight);
            }
            exit(EXIT_FAILURE);
        }
        sprintf(parser.argWidth, "%d", options->width);
        parser.dynamicWidth = true;
    }

    return parser;
}

void cleanupArgParser(ArgParserAdt *parser) {
    if (parser == NULL) {
        return;
    }

    // Free dynamic width/height strings
    if (parser->dynamicWidth && parser->argWidth != NULL) {
        free(parser->argWidth);
        parser->argWidth = NULL;
    }

    if (parser->dynamicHeight && parser->argHeight != NULL) {
        free(parser->argHeight);
        parser->argHeight = NULL;
    }
} 