CC = gcc
CFLAGS = -Wall -pedantic -I./libraries
LDFLAGS = -pthread -lrt

SRC_DIR = .
EXEC_DIR = executables
LIB_DIR = libraries

# Source files
MASTER_SRC = master/master.c
PLAYER_SRC = player/player.c
VIEW_SRC = view/viewVieja.c
SHARED_MEMORY_SRC = $(LIB_DIR)/sharedMemory.c

# Object files
MASTER_OBJ = $(MASTER_SRC:.c=.o)
PLAYER_OBJ = $(PLAYER_SRC:.c=.o)
VIEW_OBJ = $(VIEW_SRC:.c=.o)
SHARED_MEMORY_OBJ = $(SHARED_MEMORY_SRC:.c=.o)

# Binary files
MASTER_BIN = $(EXEC_DIR)/master
PLAYER_BIN = $(EXEC_DIR)/player
VIEW_BIN = $(EXEC_DIR)/viewVieja

all: $(MASTER_BIN) $(PLAYER_BIN) $(VIEW_BIN)

$(EXEC_DIR)/master: $(MASTER_OBJ) $(SHARED_MEMORY_OBJ)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(MASTER_OBJ) $(SHARED_MEMORY_OBJ) -o $@ $(LDFLAGS)

$(EXEC_DIR)/player: $(PLAYER_OBJ) $(SHARED_MEMORY_OBJ)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(PLAYER_OBJ) $(SHARED_MEMORY_OBJ) -o $@ $(LDFLAGS)

$(EXEC_DIR)/viewVieja: $(VIEW_OBJ) $(SHARED_MEMORY_OBJ)
	@mkdir -p $(EXEC_DIR)
	$(CC) $(VIEW_OBJ) $(SHARED_MEMORY_OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(EXEC_DIR)
	rm -f $(MASTER_OBJ) $(PLAYER_OBJ) $(VIEW_OBJ) $(SHARED_MEMORY_OBJ)

.PHONY: all clean 