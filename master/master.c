#include "master.h"
#include "gameStateAdt.h"
#include "gameSyncAdt.h"
#include "processManagerAdt.h"
#include "gameLogicAdt.h"
#include "argParserAdt.h"
#include "gameConfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

// Global variables for resource management and cleanup
GameStateAdt gGameStateAdt;
GameSyncAdt gGameSyncAdt;
int processReturn[MAX_PLAYERS];

/**
 * Signal handler for clean termination
 * @param sig      Signal number
 * @param exitCode Exit code to use (EXIT_SUCCESS or EXIT_FAILURE)
 */
void cleanAndEnd(int sig, int exitCode) {
    cleanupGameSync(&gGameSyncAdt);
    cleanupGameState(&gGameStateAdt);
    exit(exitCode);
}

/**
 * Signal handler wrapper for clean termination through SIGINT
 */
void signalHandler(int sig) {
    cleanAndEnd(sig, EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    // Set up signal handler for SIGINT
    signal(SIGINT, signalHandler);

    // Parse command line arguments
    Options options;
    ArgParserAdt argParser = parseArguments(argc, argv, &options);

    // Initialize game state and sync
    size_t boardSize = options.width * options.height * sizeof(int);
    gGameStateAdt = initGameState(boardSize, &options);
    gGameSyncAdt = initGameSync();

    // Initialize process manager and create processes
    ProcessManagerAdt processManager = initProcessManager(options.num_players);
    createPipes(&processManager);
    createProcesses(&processManager, options.view_path, options.player_paths, 
                   gGameStateAdt.state, argParser.argWidth, argParser.argHeight);
    
    // Free dynamic argument strings
    cleanupArgParser(&argParser);

    // Initialize game logic and run the game
    GameLogicAdt gameLogic = initGameLogic(gGameStateAdt.state, gGameSyncAdt.sync, 
                                          options.timeout_sec, options.delay_ms);
    distributePlayers(&gameLogic);
    printOptions(&options);
    
    sleep(1); // Small delay before starting

    // Run the main game loop
    runGameLoop(&gameLogic, &processManager);

    // Wait for all player processes and collect results
    waitForPlayers(&processManager, processReturn);

    // Print final results and clean up
    printFinalResults(&gameLogic, processReturn);
    cleanupProcessManager(&processManager);
    cleanupGameState(&gGameStateAdt);
    cleanupGameSync(&gGameSyncAdt);

    cleanAndEnd(0, EXIT_SUCCESS);
    
    return 0;
} 