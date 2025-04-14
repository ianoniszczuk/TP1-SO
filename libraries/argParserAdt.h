#ifndef ARG_PARSER_ADT_H
#define ARG_PARSER_ADT_H

#include "gameConfig.h"
#include <stdbool.h>

#define MAX_DIMENSIONS_LEN 20

typedef struct {
    char *argWidth;         
    char *argHeight;        
} ArgParserAdt;

/**
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @param options Options structure to populate
 * @return ArgParserAdt structure with argument values
 */
ArgParserAdt parseArguments(int argc, char *argv[], Options *options);

#endif 