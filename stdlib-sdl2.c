#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>

#include "chip8.h"

#define SCALE_MULTIPLY 10

SDL_Renderer *renderer;
SDL_Window *window;
SDL_Texture *texture;
SDL_Event e;

uint8_t InitGraphics(void)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("CHIP-8",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      SCREEN_WIDTH*SCALE_MULTIPLY,
      SCREEN_HEIGHT*SCALE_MULTIPLY,
      SDL_WINDOW_SHOWN);
    
    renderer = SDL_CreateRenderer(window,
    -1,
    SDL_RENDERER_ACCELERATED);
    
    texture = SDL_CreateTexture(renderer,
      SDL_PIXELFORMAT_ARGB8888,
      SDL_TEXTUREACCESS_STATIC,
      SCREEN_WIDTH,
      SCREEN_HEIGHT);
    
    return EXIT_SUCCESS;
}

uint8_t StopGraphics(void)
{
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return EXIT_SUCCESS;
}

static unsigned char keyboard_scancodes[16] = {
  0X1B, /* X == 0 */
  0x1E, /* 1 == 1 */
  0x1F, /* 2 == 2 */
  0x20, /* 3 == 3 */
  0x14, /* Q == 4 */
  0x1A, /* W == 5 */
  0x08, /* E == 6 */
  0x04, /* A == 7 */
  0x16, /* S == 8 */
  0x07, /* D == 9 */
  0X1D, /* Z == A */
  0X06, /* C == B */
  0x21, /* 4 == C */
  0x15, /* R == D */
  0x09, /* F == E */
  0X19  /* V == F */
};

static uint8_t setKeyInfo(SDL_KeyboardEvent *keyboard)
{
  for(uint8_t i = 0; i < sizeof(key); i++)
  {
    if(keyboard->keysym.scancode == keyboard_scancodes[i])
      key[i] = (keyboard->type == SDL_KEYDOWN) ? 1:0;
  }
  return EXIT_SUCCESS; 
}

uint8_t UpdateInput(void)
{
  uint8_t rc = 0;
  while( SDL_PollEvent( &e ) != 0 ) {
    switch(e.type) {
      case SDL_QUIT:
        rc = EXIT_FAILURE; 
        break;
     case SDL_KEYDOWN:
      /* fall-through*/
     case SDL_KEYUP:
      rc = setKeyInfo(&e.key);
      break;
    }
  }
  return rc;
}

uint8_t PlayBeep(void)
{
  printf("%c", 7);
  return EXIT_SUCCESS;
}

uint8_t InitPlatform(void)
{
  srand( (unsigned)time( NULL ) );
  return EXIT_SUCCESS;
}

uint8_t LoadGame(char* filename)
{
    uint32_t size;
    FILE* fptr;
    fptr = fopen(filename, "rb");
    if(fptr == NULL)
    {
        printf("Failed to open game!\r\n");
        return -1;
    }
    fseek(fptr, 0, SEEK_END);
    size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);
    
    if(size > GAME_SIZE) {
        printf("Game too big!\r\n");
        return -1;
    }
    fread(&memory[512], size, 1, fptr);
    
    fclose(fptr);
    return 0;
}


/*
 * DrawGraphics
 *
 * I originially wrote to the screen one pixel at a time; However this was
 * very slow, so I reworked the code to instead create a texture and to map
 * the 2d graphics array to the pixel plane. Still not the fastest but MUCH
 * better than it was.
 */
uint8_t DrawGraphics(void)
{
  uint32_t pixels[SCREEN_WIDTH*SCREEN_HEIGHT] = {0};
  
  uint32_t k = 0;
  
#ifdef SMALL_GFX
  for(uint8_t height = 0; height < SCREEN_HEIGHT; height++)
  {
    for(uint8_t width = 0; width < 8; width++)
    {
      for(uint8_t bit = 8; bit > 0; bit--)
      {
        pixels[k++] = (((gfx_small[width][height] >> (bit-1)) & 1) != 0) ? 0xFFFFFFFF:0xFF000000;
      }
    }
  }
#else /* SMALL_GFX */
  for(uint8_t height = 0; height < SCREEN_HEIGHT; height++)
  {
    for(uint8_t width = 0; width < SCREEN_WIDTH; width++)
    {
      pixels[k++] = (gfx[width][height] != 0) ? 0xFFFFFFFF:0xFF000000;
    }
  }
#endif
  
  
  SDL_UpdateTexture(texture, NULL, pixels,
                    SCREEN_WIDTH * sizeof(uint32_t));
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
  return EXIT_SUCCESS;
}