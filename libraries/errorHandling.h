#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include <stdio.h>
#include <stdlib.h>

#define ERROR_EXIT(msg)     do { perror(msg); exit(EXIT_FAILURE); } while (0)

#endif
