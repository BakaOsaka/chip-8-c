CROSS_COMPILE := x86_64-w64-mingw32-
SDL_DIR := x86_64-w64-mingw32/include
MINGW_DIR := x86_64-w64-mingw32/bin

CFLAGS += -Wall -O2 -Wimplicit-fallthrough

all : main.exe

clean:
	rm -f *.exe

#---------------------------------

main.exe : main.c stdlib-sdl2.c chip8.h
	$(CROSS_COMPILE)gcc $(CFLAGS) main.c stdlib-sdl2.c -I$(SDL_DIR) -L$(MINGW_DIR) -lsdl2 -o main

