#ifndef PROCESS_MANAGER_ADT_H
#define PROCESS_MANAGER_ADT_H

#include "sharedHeaders.h"
#include <stdbool.h>
#include <sys/types.h>

typedef struct {
    int (*pipes)[2];          
    int numPlayers;           
    pid_t *playerPids;       
    pid_t viewPid;           
    bool hasView;             
} ProcessManagerAdt;

/**
 * @param numPlayers Number of player processes to manage
 * @return ProcessManagerAdt structure
 */
ProcessManagerAdt initProcessManager(int numPlayers);

/**
 * @param pm ProcessManagerAdt structure
 */
void createPipes(ProcessManagerAdt *pm);

/**
 * @param pm ProcessManagerAdt structure
 * @param viewPath Path to view executable
 * @param playerPaths Array of paths to player executables
 * @param state GameState structure
 * @param argWidth Width argument as string
 * @param argHeight Height argument as string
 */
void createProcesses(ProcessManagerAdt *pm, char *viewPath, char *playerPaths[], 
                    GameState *state, char *argWidth, char *argHeight);

/**
 * @param pm ProcessManagerAdt structure
 * @param returnCodes Array to store player return codes
 */
void waitForPlayers(ProcessManagerAdt *pm, int returnCodes[]);

/**
 * @param pm ProcessManagerAdt structure
 */
void cleanupProcessManager(ProcessManagerAdt *pm);

/**
 * @param pm ProcessManagerAdt structure
 * @param playerIndex Index of the player whose pipe should be closed
 */
void closePlayerPipe(ProcessManagerAdt *pm, int playerIndex);

#endif 