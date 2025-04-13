#!/bin/bash

# Check if valgrind flag is passed
if [ "$1" = "--valgrind" ]; then
  # Run with Valgrind for memory leak detection
  valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./executables/master -d 400 -t 5 -p ./executables/player ./executables/player -v ./executables/viewVieja
else
  # Normal run
  ./executables/master -d 400 -t 20 -p ./executables/player ./executables/player ./executables/player ./executables/player ./executables/player -v ./executables/view
fi