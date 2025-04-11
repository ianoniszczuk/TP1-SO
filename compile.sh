#!/bin/bash

# Create executables directory if it doesn't exist
mkdir -p executables

# Compile shared memory and ADT libraries first
gcc -Wall -pedantic -I./libraries -c ./libraries/sharedMemoryAdt.c -o ./executables/sharedMemoryAdt.o
gcc -Wall -pedantic -I./libraries -c ./libraries/gameStateAdt.c -o ./executables/gameStateAdt.o
gcc -Wall -pedantic -I./libraries -c ./libraries/gameSyncAdt.c -o ./executables/gameSyncAdt.o

# Compile view
gcc -Wall -pedantic -I./libraries ./view/view.c ./executables/sharedMemoryAdt.o -o executables/view -pthread -lrt

# Compile player
gcc -Wall -pedantic -I./libraries ./player/player.c ./executables/sharedMemoryAdt.o -o executables/player -pthread -lrt

# Compile master
gcc -Wall -pedantic -I./libraries ./master/master.c ./executables/sharedMemoryAdt.o ./executables/gameStateAdt.o ./executables/gameSyncAdt.o -o executables/master -pthread -lrt

# Compile old view (if needed)
gcc -Wall -pedantic -I./libraries ./view/viewVieja.c ./executables/sharedMemoryAdt.o -o executables/viewVieja -pthread -lrt

# Clean up object files
rm -f ./executables/*.o