#ifndef GAME_LOGIC_ADT_H
#define GAME_LOGIC_ADT_H

#include "sharedHeaders.h"
#include "processManagerAdt.h"
#include "errorHandling.h"
#include <stdbool.h>

typedef struct {
    GameState *state;        
    GameSync *sync;          
    int timeout;            
    int delayMs;             
} GameLogicAdt;

/**
 * @param state Pointer to game state
 * @param sync Pointer to game synchronization
 * @param timeout Timeout in seconds
 * @param delayMs Delay between moves in milliseconds
 * @return GameLogicAdt structure
 */
GameLogicAdt initGameLogic(GameState *state, GameSync *sync, int timeout, int delayMs);

/**
 * @param logic GameLogicAdt structure
 */
void distributePlayers(GameLogicAdt *logic);

/**
 * @param logic GameLogicAdt structure
 * @param pm ProcessManagerAdt structure
 */
void runGameLoop(GameLogicAdt *logic, ProcessManagerAdt *pm);

/**
 * @param logic GameLogicAdt structure
 * @param returnCodes Array of player return codes
 */
void printFinalResults(GameLogicAdt *logic, int returnCodes[]);

#endif 