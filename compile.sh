#!/bin/bash

# Create executables directory if it doesn't exist
mkdir -p executables

# Compile shared memory library first
gcc -Wall -pedantic -I./libraries -c ./libraries/sharedMemory.c -o ./libraries/sharedMemory.o

# Compile view
gcc -Wall -pedantic -I./libraries ./view/view.c ./libraries/sharedMemory.o -o executables/view -pthread -lrt

# Compile player
gcc -Wall -pedantic -I./libraries ./player/player.c ./libraries/sharedMemory.o -o executables/player -pthread -lrt

# Compile master
gcc -Wall -pedantic -I./libraries ./master/master.c ./libraries/sharedMemory.o -o executables/master -pthread -lrt

# Compile old view (if needed)
gcc -Wall -pedantic -I./libraries ./view/viewVieja.c ./libraries/sharedMemory.o -o executables/viewVieja -pthread -lrt

# Clean up object files
rm -f ./libraries/sharedMemory.o