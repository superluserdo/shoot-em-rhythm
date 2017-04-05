output: main.o music2.o clock.o startscreen.o level.o pause.o
	gcc -g3 main.o music.o clock.o startscreen.o level.o pause.o -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -pthread -lconfig -o ../bin/shoot-em-rhythm -Wall

main.o: main.c main.h
	gcc -c main.c

music2.o: music.c
	gcc -c music.c

clock.o: clock.c
	gcc -c clock.c

startscreen.o: startscreen.c
	gcc -c startscreen.c

level.o: level.c main.h level.h
	gcc -c level.c

pause.o: pause.c
	gcc -c pause.c

