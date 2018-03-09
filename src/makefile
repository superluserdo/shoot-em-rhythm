output: main.o music.o clock.o startscreen.o level.o pause.o helpers.o animate.o transform.o rdtsc.o
	gcc -g3 main.o music.o clock.o startscreen.o level.o pause.o helpers.o animate.o transform.o rdtsc.o -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lpthread -lconfig -lm -o ../bin/shoot-em-rhythm -Wall

main.o: main.c structdef.h main.h music.h
	gcc -g3 -c main.c

music.o: music.c structdef.h music.h
	gcc -g3 -c music.c

clock.o: clock.c structdef.h clock.h
	gcc -g3 -c clock.c

startscreen.o: startscreen.c structdef.h music.h
	gcc -g3 -c startscreen.c

level.o: level.c structdef.h main.h level.h helpers.h music.h clock.h animate.h
	gcc -g3 -c level.c

pause.o: pause.c structdef.h
	gcc -g3 -c pause.c

helpers.o: helpers.c structdef.h main.h level.h helpers.h
	gcc -g3 -c helpers.c

animate.o: animate.c structdef.h animate.h
	gcc -g3 -c animate.c

transform.o: transform.c structdef.h transform.h
	gcc -g3 -c transform.c

rdtsc.o: rdtsc.c
	gcc -g3 -c rdtsc.c

clean:
	rm -f *.o
