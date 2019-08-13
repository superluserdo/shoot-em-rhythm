#!/bin/sh
echo "Building structdef.py..."
echo $(pwd)
echo '#include <SDL2/SDL.h>\n#include <SDL2/SDL_image.h>' > sdl-incl.h
cat sdl-incl.h ../src/include/structdef.h > structdef-incl.h
clang2py structdef-incl.h > ../scripts/structdef.py
##clang2py --clang-args="-v" structdef-incl.h > structdef.py
