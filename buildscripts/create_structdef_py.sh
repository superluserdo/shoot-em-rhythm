#!/bin/sh

#clang2py is broken for now so don't create python stuff:
exit 0

cd `dirname $0`
echo "Building structdef.py..."
echo $(pwd)
echo '#include <SDL2/SDL.h>\n#include <SDL2/SDL_image.h>' > sdl-incl.h
cat sdl-incl.h ../src/include/structdef.h > structdef-incl.h
clang2py structdef-incl.h > ../scripts/structdef.py
