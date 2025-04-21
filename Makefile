CC = gcc
CFLAGS = -g -Wall -pedantic -I./libraries
LDFLAGS = -pthread -lrt

SRC_DIR = .
EXEC_DIR = executables
LIB_DIR = libraries

# Source files
MASTER_SRC = master/master.c
PLAYER_SRC = player/player.c
VIEW_SRC = view/view.c
SHARED_MEMORY_SRC = libraries/sharedMemoryAdt.c
GAME_STATE_ADT_SRC = libraries/gameStateAdt.c
GAME_SYNC_ADT_SRC = libraries/gameSyncAdt.c
GAME_CONFIG_SRC = libraries/gameConfig.c
ARG_PARSER_ADT_SRC = libraries/argParserAdt.c
PROCESS_MANAGER_ADT_SRC = libraries/processManagerAdt.c
GAME_LOGIC_ADT_SRC = libraries/gameLogicAdt.c
WEST_SRC = player/westStrategy.c
RAND_SRC = player/randStrategy.c

# Object files
MASTER_OBJ = $(EXEC_DIR)/master.o
PLAYER_OBJ = $(EXEC_DIR)/player.o
VIEW_OBJ = $(EXEC_DIR)/view.o
SHARED_MEMORY_OBJ = $(EXEC_DIR)/sharedMemoryAdt.o
GAME_STATE_ADT_OBJ = $(EXEC_DIR)/gameStateAdt.o
GAME_SYNC_ADT_OBJ = $(EXEC_DIR)/gameSyncAdt.o
GAME_CONFIG_OBJ = $(EXEC_DIR)/gameConfig.o
ARG_PARSER_ADT_OBJ = $(EXEC_DIR)/argParserAdt.o
PROCESS_MANAGER_ADT_OBJ = $(EXEC_DIR)/processManagerAdt.o
GAME_LOGIC_ADT_OBJ = $(EXEC_DIR)/gameLogicAdt.o
WEST_OBJ = $(EXEC_DIR)/west.o
RAND_OBJ = $(EXEC_DIR)/rand.o

# Binary files
MASTER_BIN = $(EXEC_DIR)/master
PLAYER_BIN = $(EXEC_DIR)/player
VIEW_BIN = $(EXEC_DIR)/view

# All ADT Objects used by master
MASTER_ADT_OBJS = $(SHARED_MEMORY_OBJ) $(GAME_STATE_ADT_OBJ) $(GAME_SYNC_ADT_OBJ) \
                 $(GAME_CONFIG_OBJ) $(ARG_PARSER_ADT_OBJ) $(PROCESS_MANAGER_ADT_OBJ) \
                 $(GAME_LOGIC_ADT_OBJ)

pWest: $(PLAYER_OBJ) $(SHARED_MEMORY_OBJ) $(WEST_OBJ)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(PLAYER_OBJ) $(SHARED_MEMORY_OBJ) $(WEST_OBJ) -o $(EXEC_DIR)/pWest $(LDFLAGS)
	cp $(EXEC_DIR)/pWest $(EXEC_DIR)/player

pRand: $(PLAYER_OBJ) $(SHARED_MEMORY_OBJ) $(RAND_OBJ)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(PLAYER_OBJ) $(SHARED_MEMORY_OBJ) $(RAND_OBJ) -o $(EXEC_DIR)/pRand $(LDFLAGS)
	cp $(EXEC_DIR)/pRand $(EXEC_DIR)/player

all: $(MASTER_BIN) $(PLAYER_BIN) $(VIEW_BIN) $(VIEW_VIEJA_BIN)

$(EXEC_DIR)/master: $(MASTER_OBJ) $(MASTER_ADT_OBJS)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(MASTER_OBJ) $(MASTER_ADT_OBJS) -o $@ $(LDFLAGS)

$(EXEC_DIR)/player: $(PLAYER_OBJ) $(SHARED_MEMORY_OBJ) $(RAND_OBJ)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(PLAYER_OBJ) $(SHARED_MEMORY_OBJ) $(RAND_OBJ) -o $@ $(LDFLAGS)

$(EXEC_DIR)/view: $(VIEW_OBJ) $(SHARED_MEMORY_OBJ)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(VIEW_OBJ) $(SHARED_MEMORY_OBJ) -o $@ $(LDFLAGS)

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

$(SHARED_MEMORY_OBJ): $(SHARED_MEMORY_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(GAME_STATE_ADT_OBJ): $(GAME_STATE_ADT_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(GAME_SYNC_ADT_OBJ): $(GAME_SYNC_ADT_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(GAME_CONFIG_OBJ): $(GAME_CONFIG_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(ARG_PARSER_ADT_OBJ): $(ARG_PARSER_ADT_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(PROCESS_MANAGER_ADT_OBJ): $(PROCESS_MANAGER_ADT_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(GAME_LOGIC_ADT_OBJ): $(GAME_LOGIC_ADT_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(WEST_OBJ): $(WEST_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(RAND_OBJ): $(RAND_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(EXEC_DIR)

.PHONY: all clean