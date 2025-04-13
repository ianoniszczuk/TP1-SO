#include "processManagerAdt.h"
#include "errorHandling.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

/**
 * Helper function to create argument arrays
 * 
 * @param programPath Program path to use as first argument
 * @param argWidth Width argument as string
 * @param argHeight Height argument as string
 * @return Allocated argument array, must be freed with freeArgs
 */
static char** createArgs(const char *programPath, const char *argWidth, const char *argHeight) {
    char **args = malloc(4 * sizeof(char *));
    if (!args) {
        return NULL;
    }
    
    args[0] = strdup(programPath);
    args[1] = strdup(argWidth);
    args[2] = strdup(argHeight);
    args[3] = NULL;
    
    if (!args[0] || !args[1] || !args[2]) {
        for (int i = 0; i < 3; i++) {
            if (args[i]) free(args[i]);
        }
        free(args);
        return NULL;
    }
    
    return args;
}

/**
 * Helper function to free argument arrays
 * 
 * @param args Argument array to free
 */
static void freeArgs(char **args) {
    if (!args) return;
    
    for (int i = 0; i < 3; i++) {
        if (args[i]) free(args[i]);
    }
    free(args);
}

ProcessManagerAdt initProcessManager(int numPlayers) {
    ProcessManagerAdt pm;
    
    // Allocate memory for pipes
    pm.pipes = malloc(numPlayers * sizeof(int[2]));
    if (!pm.pipes) {
        ERROR_EXIT("Error allocating memory for pipes");
    }
    
    // Allocate memory for player PIDs
    pm.playerPids = malloc(numPlayers * sizeof(pid_t));
    if (!pm.playerPids) {
        free(pm.pipes);
        ERROR_EXIT("Error allocating memory for player PIDs");
    }
    
    pm.numPlayers = numPlayers;
    pm.viewPid = -1;
    pm.hasView = false;
    
    return pm;
}

void createPipes(ProcessManagerAdt *pm) {
    for (int i = 0; i < pm->numPlayers; i++) {
        if (pipe(pm->pipes[i]) == -1) {
            cleanupProcessManager(pm);
            ERROR_EXIT("Error creating pipes");
        }
    }
}

/**
 * Helper function to create a view process
 * 
 * @param pm Process manager
 * @param viewPath Path to view executable
 * @param argWidth Width argument as string
 * @param argHeight Height argument as string
 * @return true on success, false on failure
 */
static bool createViewProcess(ProcessManagerAdt *pm, char *viewPath, char *argWidth, char *argHeight) {
    if (viewPath == NULL) {
        return true;  // No view to create, not an error
    }
    
    char **args = createArgs(viewPath, argWidth, argHeight);
    if (!args) {
        return false;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        freeArgs(args);
        return false;
    } else if (pid == 0) {
        // Child process
        execve(viewPath, args, NULL);
        ERROR_EXIT("Error in execve (view)");
    }
    
    // Parent process
    pm->viewPid = pid;
    pm->hasView = true;
    freeArgs(args);
    return true;
}

/**
 * Helper function to create a player process
 * 
 * @param pm Process manager
 * @param state Game state to update with player PID
 * @param playerPath Path to player executable
 * @param argWidth Width argument as string
 * @param argHeight Height argument as string
 * @param playerIndex Index of the player
 * @return true on success, false on failure
 */
static bool createPlayerProcess(ProcessManagerAdt *pm, GameState *state, char *playerPath, 
                               char *argWidth, char *argHeight, int playerIndex) {
    char **args = createArgs(playerPath, argWidth, argHeight);
    if (!args) {
        return false;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        freeArgs(args);
        return false;
    } else if (pid == 0) {
        // Child process - close unused pipe ends
        for (int j = 0; j < pm->numPlayers; j++) {
            close(pm->pipes[j][0]);
            if (playerIndex != j) {
                close(pm->pipes[j][1]);
            }
        }

        if (dup2(pm->pipes[playerIndex][1], STDOUT_FILENO) == -1) {
            ERROR_EXIT("Error in dup2");
        }

        execve(playerPath, args, NULL);
        ERROR_EXIT("Error in execve (player)");
    }
    
    // Parent process
    close(pm->pipes[playerIndex][1]);
    pm->playerPids[playerIndex] = pid;
    state->players[playerIndex].pid = pid;
    freeArgs(args);
    return true;
}

void createProcesses(ProcessManagerAdt *pm, char *viewPath, char *playerPaths[], 
                     GameState *state, char *argWidth, char *argHeight) {
    // Create view process if specified
    if (viewPath != NULL) {
        if (!createViewProcess(pm, viewPath, argWidth, argHeight)) {
            cleanupProcessManager(pm);
            ERROR_EXIT("Error creating view process");
        }
    }
    
    // Create player processes
    for (int i = 0; i < pm->numPlayers; i++) {
        if (!createPlayerProcess(pm, state, playerPaths[i], argWidth, argHeight, i)) {
            cleanupProcessManager(pm);
            ERROR_EXIT("Error creating player process");
        }
    }
}

void waitForPlayers(ProcessManagerAdt *pm, int returnCodes[]) {
    int status;
    for (int i = 0; i < pm->numPlayers; i++) {
        pid_t pid = waitpid(pm->playerPids[i], &status, 0);
        if (pid > 0) {
            if (WIFEXITED(status)) {
                returnCodes[i] = WEXITSTATUS(status);
            }
        } else {
            returnCodes[i] = -1;
        }
    }
}

void cleanupProcessManager(ProcessManagerAdt *pm) {
    if (pm == NULL) {
        return;
    }
    
    // Close all open pipes
    if (pm->pipes) {
        for (int i = 0; i < pm->numPlayers; i++) {
            close(pm->pipes[i][0]); // Close read end
        }
        free(pm->pipes);
        pm->pipes = NULL;
    }
    
    // Free player PIDs array
    if (pm->playerPids) {
        free(pm->playerPids);
        pm->playerPids = NULL;
    }
}

/**
 * Closes the pipe for a specific player.
 * This is used when a player becomes blocked to stop receiving further moves.
 * 
 * @param pm ProcessManagerAdt structure
 * @param playerIndex Index of the player whose pipe should be closed
 */
void closePlayerPipe(ProcessManagerAdt *pm, int playerIndex) {
    if (!pm || playerIndex < 0 || playerIndex >= pm->numPlayers) {
        return; // Invalid parameters
    }
    
    // Check if pipe is still open
    if (pm->pipes[playerIndex][0] > 0) {
        // Close the read end of the pipe
        close(pm->pipes[playerIndex][0]);
        pm->pipes[playerIndex][0] = -1; // Mark as closed
        
        printf("Pipe closed for player %d (blocked)\n", playerIndex);
    }
} 