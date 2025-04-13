#ifndef GAME_LOGIC_ADT_H
#define GAME_LOGIC_ADT_H

#include "sharedHeaders.h"
#include "processManagerAdt.h"
#include "errorHandling.h"
#include <stdbool.h>

/**
 * Structure to manage game logic
 */
typedef struct {
    GameState *state;        // Pointer to game state
    GameSync *sync;          // Pointer to game sync
    int timeout;             // Timeout in seconds
    int delayMs;             // Delay between moves in milliseconds
} GameLogicAdt;

/**
 * Initializes the game logic
 * 
 * @param state Pointer to game state
 * @param sync Pointer to game synchronization
 * @param timeout Timeout in seconds
 * @param delayMs Delay between moves in milliseconds
 * @return GameLogicAdt structure
 */
GameLogicAdt initGameLogic(GameState *state, GameSync *sync, int timeout, int delayMs);

/**
 * Distributes players on the game board
 * 
 * @param logic GameLogicAdt structure
 */
void distributePlayers(GameLogicAdt *logic);

/**
 * Runs the main game loop, handling player movements
 * 
 * @param logic GameLogicAdt structure
 * @param pm ProcessManagerAdt structure
 */
void runGameLoop(GameLogicAdt *logic, ProcessManagerAdt *pm);

/**
 * Prints final game results
 * 
 * @param logic GameLogicAdt structure
 * @param returnCodes Array of player return codes
 */
void printFinalResults(GameLogicAdt *logic, int returnCodes[]);

#endif 