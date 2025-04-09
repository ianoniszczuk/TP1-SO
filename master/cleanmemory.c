#include "master.h"

#define GAME_STATE "/game_state"
#define GAME_SYNC "/game_sync"

int main(){

    GameState *state;
    GameSync *sync; 
    int width = 10;
    int height = 10;
    size_t board_size = width * height * sizeof(int);

    int state_size = sizeof(GameState) + board_size;


     // Destruir semáforos
    sem_destroy(&sync->printNeeded);
    sem_destroy(&sync->printDone);
    sem_destroy(&sync->turnstile);
    sem_destroy(&sync->resourceAccess);
    sem_destroy(&sync->readerCountMutex);
    // Desmapear memorias compartidas
    munmap(state, state_size);
    munmap(sync, sizeof(GameSync));
    // Eliminar objetos de memoria compartida
    shm_unlink(GAME_STATE);
    shm_unlink(GAME_SYNC);
}