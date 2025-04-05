gcc -Wall -pedantic ./view/view.c -o executables/view
gcc -Wall -pedantic ./player/player.c -o executables/player
gcc -Wall -pedantic ./master/master.c -o executables/master

./executables/master.o -p ./executables/player ./executables/player ./executables/player ./executables/player -v ./executables/view