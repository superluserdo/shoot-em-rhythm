output: main.o music.o clock.o startscreen.o level.o pause.o helpers.o
	gcc -g3 main.o music.o clock.o startscreen.o level.o pause.o helpers.o -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -pthread -lconfig -o ../bin/shoot-em-rhythm -Wall

main.o: main.c main.h music.h
	gcc -c main.c

music.o: music.c music.h
	gcc -c music.c

clock.o: clock.c
	gcc -c clock.c

startscreen.o: startscreen.c music.h
	gcc -c startscreen.c

level.o: level.c main.h level.h helpers.h music.h
	gcc -c level.c

pause.o: pause.c
	gcc -c pause.c

helpers.o: helpers.c main.h level.h helpers.h
	gcc -c helpers.c

clean:
	rm -f *.o
