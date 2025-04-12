#ifndef PROCESS_MANAGER_ADT_H
#define PROCESS_MANAGER_ADT_H

#include "sharedHeaders.h"
#include <stdbool.h>
#include <sys/types.h>

/**
 * Structure to manage game processes and pipes
 */
typedef struct {
    int (*pipes)[2];          // Pipe array for communication with players
    int numPlayers;           // Number of player processes
    pid_t *playerPids;        // Array of player process IDs
    pid_t viewPid;            // View process ID
    bool hasView;             // Flag indicating if view process exists
} ProcessManagerAdt;

/**
 * Initializes the process manager
 * 
 * @param numPlayers Number of player processes to manage
 * @return ProcessManagerAdt structure
 */
ProcessManagerAdt initProcessManager(int numPlayers);

/**
 * Creates all necessary pipes for player communication
 * 
 * @param pm ProcessManagerAdt structure
 */
void createPipes(ProcessManagerAdt *pm);

/**
 * Creates view and player processes
 * 
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
 * Waits for all player processes to terminate and collects their exit status
 * 
 * @param pm ProcessManagerAdt structure
 * @param returnCodes Array to store player return codes
 */
void waitForPlayers(ProcessManagerAdt *pm, int returnCodes[]);

/**
 * Cleans up resources used by the process manager
 * 
 * @param pm ProcessManagerAdt structure
 */
void cleanupProcessManager(ProcessManagerAdt *pm);

#endif 