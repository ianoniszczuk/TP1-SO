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
SHARED_MEMORY_SRC = libraries/sharedMemoryAdt.c

# Object files
MASTER_OBJ = $(EXEC_DIR)/master.o
PLAYER_OBJ = $(EXEC_DIR)/player.o
VIEW_OBJ = $(EXEC_DIR)/view.o
SHARED_MEMORY_OBJ = $(EXEC_DIR)/sharedMemoryAdt.o

# Binary files
MASTER_BIN = $(EXEC_DIR)/master
PLAYER_BIN = $(EXEC_DIR)/player
VIEW_BIN = $(EXEC_DIR)/view

all: $(MASTER_BIN) $(PLAYER_BIN) $(VIEW_BIN)

$(EXEC_DIR)/master: $(MASTER_OBJ) $(SHARED_MEMORY_OBJ)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(MASTER_OBJ) $(SHARED_MEMORY_OBJ) -o $@ $(LDFLAGS)

$(EXEC_DIR)/player: $(PLAYER_OBJ) $(SHARED_MEMORY_OBJ)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(PLAYER_OBJ) $(SHARED_MEMORY_OBJ) -o $@ $(LDFLAGS)

$(EXEC_DIR)/view: $(VIEW_OBJ) $(SHARED_MEMORY_OBJ)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(VIEW_OBJ) $(SHARED_MEMORY_OBJ) -o $@ $(LDFLAGS)

$(EXEC_DIR)/master.o: $(MASTER_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(EXEC_DIR)/player.o: $(PLAYER_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(EXEC_DIR)/view.o: $(VIEW_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(EXEC_DIR)/sharedMemoryAdt.o: $(SHARED_MEMORY_SRC)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(EXEC_DIR)

.PHONY: all clean