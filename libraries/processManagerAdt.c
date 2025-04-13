#include "processManagerAdt.h"
#include "errorHandling.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

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

void createProcesses(ProcessManagerAdt *pm, char *viewPath, char *playerPaths[], 
                     GameState *state, char *argWidth, char *argHeight) {
    pid_t pid;
    
    // Create dynamically allocated copies of arguments - argv[0] should be program name
    char **args = malloc(4 * sizeof(char *));
    if (!args) {
        cleanupProcessManager(pm);
        ERROR_EXIT("Error allocating memory for args");
    }
    
    // For the view
    if (viewPath != NULL) {
        args[0] = strdup(viewPath);  // Program name
        args[1] = strdup(argWidth);  // First arg (width)
        args[2] = strdup(argHeight); // Second arg (height)
        args[3] = NULL;              // NULL terminator
        
        if (!args[0] || !args[1] || !args[2]) {
            
            for (int i = 0; i < 3; i++)
                if (args[i]) free(args[i]);
            free(args);
            cleanupProcessManager(pm);
            ERROR_EXIT("Error duplicating view argument strings");
        }

        pid = fork();
        if (pid == -1) {
            for (int i = 0; i < 3; i++)
                if (args[i]) free(args[i]);
            free(args);
            cleanupProcessManager(pm);
            perror("Error in fork");
        }
        else if (pid == 0) {
            // Child process will call execve, so we don't need to free in this branch
            execve(viewPath, args, NULL);
            ERROR_EXIT("Error en execve (view)");
        }
        
        // Store view process ID
        pm->viewPid = pid;
        pm->hasView = true;
        
        // Free view args in parent
        for (int i = 0; i < 3; i++)
            if (args[i]) free(args[i]);
    }

    for (int i = 0; i < pm->numPlayers; i++) {
        // Reallocate args for each player process
        args[0] = strdup(playerPaths[i]); // Program name
        args[1] = strdup(argWidth);       // First arg (width)
        args[2] = strdup(argHeight);      // Second arg (height)
        args[3] = NULL;                   // NULL terminator
        
        if (!args[0] || !args[1] || !args[2]) {
            for (int i = 0; i < 3; i++)
                if (args[i]) free(args[i]);
            free(args);
            cleanupProcessManager(pm);
            ERROR_EXIT("Error duplicating player argument strings");
        }

        pid = fork();
        if (pid == -1) {
            for (int i = 0; i < 3; i++)
                if (args[i]) free(args[i]);
            free(args);
            cleanupProcessManager(pm);
            ERROR_EXIT("Error en fork");
        }
        else if (pid == 0) {
            // Close unused pipe ends in child
            for (int j = 0; j < pm->numPlayers; j++) {
                close(pm->pipes[j][0]);
                if (i != j) {
                    close(pm->pipes[j][1]);
                }
            }

            if (dup2(pm->pipes[i][1], STDOUT_FILENO) == -1) {
                ERROR_EXIT("Error in dup2");
            }

            execve(playerPaths[i], args, NULL);
            ERROR_EXIT("Error en execve (player)");
        } else {
            close(pm->pipes[i][1]);
            pm->playerPids[i] = pid;
            state->players[i].pid = pid;
            
            // Free player args in parent after fork
            for (int j = 0; j < 3; j++)
                if (args[j]) free(args[j]);
        }
    }
    
    // Free args array
    free(args);
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