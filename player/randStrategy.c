#include "decidePlay.h"
#include <time.h>
#include <stdlib.h>

unsigned char decidePlay(GameState *game, int playerNumber) {
    return (unsigned char)(rand() % 8);
}