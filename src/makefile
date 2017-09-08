output: main.o music.o clock.o startscreen.o level.o pause.o helpers.o animate.o
	gcc -g3 main.o music.o clock.o startscreen.o level.o pause.o helpers.o animate.o -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lpthread -lconfig -o ../bin/shoot-em-rhythm -Wall

main.o: main.c main.h music.h
	gcc -g3 -c main.c

music.o: music.c music.h
	gcc -g3 -c music.c

clock.o: clock.c clock.h
	gcc -g3 -c clock.c

startscreen.o: startscreen.c music.h
	gcc -g3 -c startscreen.c

level.o: level.c main.h level.h helpers.h music.h clock.h animate.h
	gcc -g3 -c level.c

pause.o: pause.c
	gcc -g3 -c pause.c

helpers.o: helpers.c main.h level.h helpers.h
	gcc -g3 -c helpers.c

animate.o: animate.c animate.h
	gcc -g3 -c animate.c

clean:
	rm -f *.o
