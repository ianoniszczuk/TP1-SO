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

GameStateAdt gGameStateAdt;
GameSyncAdt gGameSyncAdt;
int processReturn[MAX_PLAYERS];

void cleanAndEnd(int sig, int exitCode) {
    cleanupGameSync(&gGameSyncAdt);
    cleanupGameState(&gGameStateAdt);
    exit(exitCode);
}

void signalHandler(int sig) {
    cleanAndEnd(sig, EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, signalHandler);

    Options options;
    ArgParserAdt argParser = parseArguments(argc, argv, &options);

    size_t boardSize = options.width * options.height * sizeof(int);
    gGameStateAdt = initGameState(boardSize, &options);
    gGameSyncAdt = initGameSync();

    ProcessManagerAdt processManager = initProcessManager(options.numPlayers);
    createPipes(&processManager);
    createProcesses(&processManager, options.viewPath, options.playerPaths, 
                   gGameStateAdt.state, argParser.argWidth, argParser.argHeight);
    
    GameLogicAdt gameLogic = initGameLogic(gGameStateAdt.state, gGameSyncAdt.sync, 
                                          options.timeoutSec, options.delayMs);
    distributePlayers(&gameLogic);
    printOptions(&options);
    
    sleep(1);

    runGameLoop(&gameLogic, &processManager);

    waitForPlayers(&processManager, processReturn);

    printFinalResults(&gameLogic, processReturn);
    cleanupProcessManager(&processManager);
    cleanupGameState(&gGameStateAdt);
    cleanupGameSync(&gGameSyncAdt);

    cleanAndEnd(0, EXIT_SUCCESS);
    
    return EXIT_SUCCESS;
} 