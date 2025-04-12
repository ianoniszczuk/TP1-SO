#ifndef ARG_PARSER_ADT_H
#define ARG_PARSER_ADT_H

#include "gameConfig.h"
#include <stdbool.h>

/**
 * Structure to manage command line argument parsing
 */
typedef struct {
    char *argWidth;          // Width argument as string
    char *argHeight;         // Height argument as string
    bool dynamicWidth;       // Whether width was dynamically allocated
    bool dynamicHeight;      // Whether height was dynamically allocated
} ArgParserAdt;

/**
 * Parses command line arguments
 * 
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @param options Options structure to populate
 * @return ArgParserAdt structure with dynamic argument values
 */
ArgParserAdt parseArguments(int argc, char *argv[], Options *options);

/**
 * Frees resources used by the argument parser
 * 
 * @param parser ArgParserAdt structure
 */
void cleanupArgParser(ArgParserAdt *parser);

#endif 