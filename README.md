# chip-8-c

A simple CHIP-8 emulator written in C, with an SDL2 interface. Emulates all
required opcodes and handles graphics/input via SDL2. Currently tested on
Windows compiled with MinGW but no reason that it couldn't work on other
platforms.

To build, run `make`. The Makefile definitions `SDL_DIR` and `MINGW_DIR` can
be modified if you are building for different targets, or will default to 
local directories to the build.

### TODOs

- [ ] Fix-up collision when building with SMALL_GFX CFLAG
- [ ] Abstract main function to support non-Windows
- [ ] Better split stlib-sdl2.c in to platform-based compilation units
- [ ] Improve memory management and performance for low-power targets