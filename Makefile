CC = gcc
CFLAGS = -Wall -pedantic -I./libraries
LDFLAGS = -pthread -lrt

SRC_DIR = .
EXEC_DIR = executables
LIB_DIR = libraries

# Source files
MASTER_SRC = master/master.c
PLAYER_SRC = player/player.c
VIEW_SRC = view/view.c
VIEW_VIEJA_SRC = view/viewVieja.c
SHARED_MEMORY_SRC = libraries/sharedMemoryAdt.c
GAME_STATE_ADT_SRC = libraries/gameStateAdt.c
GAME_SYNC_ADT_SRC = libraries/gameSyncAdt.c

# Object files
MASTER_OBJ = $(EXEC_DIR)/master.o
PLAYER_OBJ = $(EXEC_DIR)/player.o
VIEW_OBJ = $(EXEC_DIR)/view.o
VIEW_VIEJA_OBJ = $(EXEC_DIR)/viewVieja.o
SHARED_MEMORY_OBJ = $(EXEC_DIR)/sharedMemoryAdt.o
GAME_STATE_ADT_OBJ = $(EXEC_DIR)/gameStateAdt.o
GAME_SYNC_ADT_OBJ = $(EXEC_DIR)/gameSyncAdt.o

# Binary files
MASTER_BIN = $(EXEC_DIR)/master
PLAYER_BIN = $(EXEC_DIR)/player
VIEW_BIN = $(EXEC_DIR)/view
VIEW_VIEJA_BIN = $(EXEC_DIR)/viewVieja

all: $(MASTER_BIN) $(PLAYER_BIN) $(VIEW_BIN) $(VIEW_VIEJA_BIN)

$(EXEC_DIR)/master: $(MASTER_OBJ) $(SHARED_MEMORY_OBJ) $(GAME_STATE_ADT_OBJ) $(GAME_SYNC_ADT_OBJ)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(MASTER_OBJ) $(SHARED_MEMORY_OBJ) $(GAME_STATE_ADT_OBJ) $(GAME_SYNC_ADT_OBJ) -o $@ $(LDFLAGS)

$(EXEC_DIR)/player: $(PLAYER_OBJ) $(SHARED_MEMORY_OBJ)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(PLAYER_OBJ) $(SHARED_MEMORY_OBJ) -o $@ $(LDFLAGS)

$(EXEC_DIR)/view: $(VIEW_OBJ) $(SHARED_MEMORY_OBJ)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(VIEW_OBJ) $(SHARED_MEMORY_OBJ) -o $@ $(LDFLAGS)

$(EXEC_DIR)/viewVieja: $(VIEW_VIEJA_OBJ) $(SHARED_MEMORY_OBJ)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(VIEW_VIEJA_OBJ) $(SHARED_MEMORY_OBJ) -o $@ $(LDFLAGS)

# Explicit rules for each object file
$(MASTER_OBJ): $(MASTER_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(PLAYER_OBJ): $(PLAYER_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(VIEW_OBJ): $(VIEW_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(VIEW_VIEJA_OBJ): $(VIEW_VIEJA_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(SHARED_MEMORY_OBJ): $(SHARED_MEMORY_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(GAME_STATE_ADT_OBJ): $(GAME_STATE_ADT_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(GAME_SYNC_ADT_OBJ): $(GAME_SYNC_ADT_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(EXEC_DIR)

.PHONY: all clean