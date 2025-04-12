#!/bin/bash

# Create executables directory if it doesn't exist
mkdir -p executables

# Compile shared memory and ADT libraries first
gcc -Wall -pedantic -I./libraries -c ./libraries/sharedMemoryAdt.c -o ./executables/sharedMemoryAdt.o
gcc -Wall -pedantic -I./libraries -c ./libraries/gameStateAdt.c -o ./executables/gameStateAdt.o
gcc -Wall -pedantic -I./libraries -c ./libraries/gameSyncAdt.c -o ./executables/gameSyncAdt.o
gcc -Wall -pedantic -I./libraries -c ./libraries/gameConfig.c -o ./executables/gameConfig.o
gcc -Wall -pedantic -I./libraries -c ./libraries/argParserAdt.c -o ./executables/argParserAdt.o
gcc -Wall -pedantic -I./libraries -c ./libraries/processManagerAdt.c -o ./executables/processManagerAdt.o
gcc -Wall -pedantic -I./libraries -c ./libraries/gameLogicAdt.c -o ./executables/gameLogicAdt.o

# Compile view
gcc -Wall -pedantic -I./libraries ./view/view.c ./executables/sharedMemoryAdt.o -o executables/view -pthread -lrt

# Compile player
gcc -Wall -pedantic -I./libraries ./player/player.c ./executables/sharedMemoryAdt.o -o executables/player -pthread -lrt

# Compile master with all ADT libraries
gcc -Wall -pedantic -I./libraries ./master/master.c \
  ./executables/sharedMemoryAdt.o \
  ./executables/gameStateAdt.o \
  ./executables/gameSyncAdt.o \
  ./executables/gameConfig.o \
  ./executables/argParserAdt.o \
  ./executables/processManagerAdt.o \
  ./executables/gameLogicAdt.o \
  -o executables/master -pthread -lrt

# Compile old view (if needed)
gcc -Wall -pedantic -I./libraries ./view/viewVieja.c ./executables/sharedMemoryAdt.o -o executables/viewVieja -pthread -lrt

# Clean up object files
rm -f ./executables/*.o