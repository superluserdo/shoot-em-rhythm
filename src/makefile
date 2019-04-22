output: main.o audio.o clock.o startscreen.o level.o spawn.o pause.o helpers.o animate.o transform.o structure.o object_logic.o dict.o rdtsc.o myfunc.mod.so structdef.py
	gcc -g3 main.o audio.o clock.o startscreen.o level.o spawn.o pause.o helpers.o animate.o transform.o structure.o object_logic.o dict.o rdtsc.o `pkg-config --cflags --libs python3` -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lpthread -lconfig -lm -ldl -o ../bin/shoot-em-rhythm -Wall -rdynamic

main.o: main.c structdef.h main.h
	gcc -g3 -c `pkg-config --cflags --libs python3` main.c

audio.o: audio.c
	gcc -g3 -c audio.c

clock.o: clock.c structdef.h clock.h
	gcc -g3 -c clock.c

startscreen.o: startscreen.c structdef.h
	gcc -g3 -c startscreen.c

level.o: level.c structdef.h main.h level.h helpers.h clock.h animate.h
	gcc -g3 -c `pkg-config --cflags --libs python3` level.c

spawn.o: spawn.c spawn.h structdef.h main.h level.h helpers.h clock.h animate.h
	gcc -g3 -c spawn.c

pause.o: pause.c structdef.h
	gcc -g3 -c pause.c

helpers.o: helpers.c structdef.h main.h level.h helpers.h
	gcc -g3 -c helpers.c

animate.o: animate.c structdef.h animate.h
	gcc -g3 -c animate.c

transform.o: transform.c structdef.h transform.h
	gcc -g3 -c transform.c

structure.o: structure.c
	gcc -g3 -c structure.c

object_logic.o: object_logic.c
	gcc -g3 -c object_logic.c

dict.o: dict.c dict.h
	gcc -g3 -c dict.c

structdef.py: structdef.h
	echo '#include <SDL2/SDL.h>\n#include <SDL2/SDL_image.h>' > sdl-incl.h
	cat sdl-incl.h structdef.h > structdef-incl.h
	clang2py structdef-incl.h > structdef.py
	#clang2py --clang-args="-v" structdef-incl.h > structdef.py

rdtsc.o: rdtsc.c
	gcc -g3 -c rdtsc.c

myfunc.mod.so: myfunc.mod.c
	gcc -g3 -c myfunc.mod.c -fPIC
	gcc -shared -o myfunc.mod.so myfunc.mod.o -fPIC

clean:
	rm -f *.o
	rm -f structdef-incl.h sdl-incl.h
