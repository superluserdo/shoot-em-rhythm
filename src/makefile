output: main.o music.o clock.o startscreen.o level.o pause.o helpers.o animate.o transform.o rdtsc.o
	gcc -g3 main.o music.o clock.o startscreen.o level.o pause.o helpers.o animate.o transform.o rdtsc.o -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lpthread -lconfig -lm -o ../bin/shoot-em-rhythm -Wall -fno-stack-protector

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

transform.o: transform.c transform.h
	gcc -g3 -c transform.c

rdtsc.o: rdtsc.c
	gcc -g3 -c rdtsc.c

clean:
	rm -f *.o
