#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
 
#include <stdio.h>
#include <stdint.h>
#include <windows.h>

#include "chip8.h"

uint16_t opcode = 0;

uint8_t game[3584] = {};

uint8_t memory[4096] = {};
uint8_t V[16] = {};
uint16_t I = 0;
uint16_t pc = 0x200;

uint8_t gfx[SCREEN_WIDTH][SCREEN_HEIGHT] = {};
uint8_t gfx_small[8][SCREEN_HEIGHT] = {};
uint8_t delay_timer = 0;
uint8_t sound_timer = 0;

uint16_t stack[16] = {};
uint16_t sp = 0;

uint8_t key[16] = {};

uint16_t draw_flag = 0;

uint8_t chip8_fontset[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0,		/* 0 */
	0x20, 0x60, 0x20, 0x20, 0x70,		/* 1 */
	0xF0, 0x10, 0xF0, 0x80, 0xF0,		/* 2 */
	0xF0, 0x10, 0xF0, 0x10, 0xF0,		/* 3 */
	0x90, 0x90, 0xF0, 0x10, 0x10,		/* 4 */
	0xF0, 0x80, 0xF0, 0x10, 0xF0,		/* 5 */
	0xF0, 0x80, 0xF0, 0x90, 0xF0,		/* 6 */
	0xF0, 0x10, 0x20, 0x40, 0x40,		/* 7 */
	0xF0, 0x90, 0xF0, 0x90, 0xF0,		/* 8 */
	0xF0, 0x90, 0xF0, 0x10, 0xF0,		/* 9 */
	0xF0, 0x90, 0xF0, 0x90, 0x90,		/* A */
	0xE0, 0x90, 0xE0, 0x90, 0xE0,		/* B */
	0xF0, 0x80, 0x80, 0x80, 0xF0,		/* C */
	0xE0, 0x90, 0x90, 0x90, 0xE0,		/* D */
	0xF0, 0x80, 0xF0, 0x80, 0xF0,		/* E */
	0xF0, 0x80, 0xF0, 0x80, 0x80		/* F */
};

void InitCPU(void)
{
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;
    
    for(uint8_t i = 0; i < sizeof(chip8_fontset); ++i)
    {
        memory[i] = chip8_fontset[i];
    }
}


void printBits(unsigned char num)
{
   for(int bit=0;bit<(sizeof(unsigned char) * 8); bit++)
   {
      printf("%i", num & 0x01);
      num = num >> 1;
   }
}

uint8_t DrawSprite(uint8_t x, uint8_t y, uint8_t height)
{
  uint8_t pixel = 0;
  V[0xF] = 0;
#ifdef SMALL_GFX
  for(uint8_t yline = 0; yline < height; yline++)
  {
    pixel = memory[I + yline];
    if((x % 8) == 0)
    {
      gfx_small[(x/8)][y + yline] ^= pixel;
    }
    else
    {
      gfx_small[(x/8)  ][y + yline] ^= ((pixel >> (  (x%8))) | (1 >> (  (x%8))) );
      gfx_small[(x/8)+1][y + yline] ^= ((pixel << (8-(x%8))) | (1 >> (8-(x%8))) );
    }    
  }
#else /* SMALL_GFX */
  for(uint8_t yline = 0; yline < height; yline++)
  {
    pixel = memory[I + yline];
    for(uint8_t xline = 0; xline < 8; xline++)
    {
      if((pixel & (0x80 >> xline)) != 0)
      {
        if(gfx[x + xline][y + yline] == 1)
          V[0xF] = 1;
        gfx[x + xline][y + yline] ^= 1;
      }
    }
  }
#endif /* SMALL_GFX */
  draw_flag = 1;
  
  return EXIT_SUCCESS;
}

uint8_t emulateCycle(void)
{
  opcode = memory[pc] << 8 | memory[pc + 1];
  printf("Opcode: 0x%04x\r\n", opcode);
    
  switch(opcode & 0xF000)
  {
    case 0x0000:
      switch(opcode & 0x00FF)
      {
        case 0x00E0:
#ifdef SMALL_GFX
          for(uint8_t yline = 0; yline < SCREEN_HEIGHT; yline++) {
            for(uint8_t xline = 0; xline < 8; xline++) {
              gfx_small[xline][yline] = 0;
            }
          }
#else /* SMALL_GFX */
          for(uint8_t yline = 0; yline < SCREEN_HEIGHT; yline++) {
            for(uint8_t xline = 0; xline < SCREEN_WIDTH; xline++) {
              gfx[xline][yline] = 0;
            }
          }
#endif /* SMALL_GFX */
          pc += 2;
          break;

        case 0x00EE:
          pc = stack[(--sp&0xF)] + 2;
          break;
        
        default:
          goto invalid_opcode;
      }
    break;
    
    case 0x1000:
      pc = (opcode & 0x0FFF);
      break;
    
    case 0x2000:
      stack[(sp++&0xF)] = pc;
      pc = (opcode & 0x0FFF);
      break;
      
    case 0x3000:
      if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
        pc += 4;
      else
        pc += 2;
      break;
      
    case 0x4000:
      if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
        pc += 4;
      else
        pc += 2;
      break;
      
    case 0x5000:
      if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x0F0) >> 4])
        pc += 4;
      else
        pc += 2;
      break;
      
    case 0x6000:
      V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
      pc += 2;
      break;
      
    case 0x7000:
      V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
      pc += 2;
      break;
      
    case 0x8000:
      switch(opcode & 0xF00F)
      {
        case 0x8000:
          V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
          pc += 2;
          break;
        
        case 0x8001:
          V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
          pc += 2;
          break;
          
        case 0x8002:
          V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
          pc += 2;
          break;
          
        case 0x8003:
          V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
          pc += 2;
          break;
        
        case 0x8004:
          if((V[(opcode & 0x0F00) >> 8] + V[(opcode & 0x00F0) >> 4]) > 255) {
            V[0xF] = 1;
          }
          else {
            V[0xF] = 0;
          }
          V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
          pc += 2;
          break;
          
        case 0x8005:
          if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]) {
            V[0xF] = 1;
          }
          else {
            V[0xF] = 0;
          }
          V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
          pc += 2;
          break;
        
        case 0x8006:
          if (V[(opcode & 0x0F00) >> 8] % 2 == 1) {
              V[0xF] = 1;
          }
          else {
              V[0xF] = 0;
          }
          V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] >> 1;
          pc += 2;
          break;
          
        case 0x8007:
          if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) {
              V[0xF] = 1;
          }
          else {
              V[0xF] = 0;
          }
          V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
          pc += 2;
          break;
          
        case 0x800E:
          V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
          V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] << 1;
          pc += 2;
          break;

        default:
          goto invalid_opcode;
      }
      break;
      
    case 0x9000:
      if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
        pc += 4;
      else
        pc += 2;
      break;
      
    case 0xA000:
      I = (opcode & 0x0FFF);
      pc += 2;
      break;

    case 0xB000:
      pc = (opcode & 0x0FFF) + V[0];
      break;
      
    case 0xC000:
      V[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
      pc += 2;
      break;
    
    case 0xD000:
      DrawSprite(V[(opcode & 0x0F00) >> 8], V[(opcode & 0x00F0) >> 4], opcode & 0x000F);
      pc += 2;
      break;
      
    case 0xE000:
      switch(opcode & 0xF0FF)
      {
        case 0xE09E:
          if(key[V[(opcode & 0x0F00) >> 8]] == 1)
            pc += 4;
          else
            pc += 2;
          break;
          
        case 0xE0A1:
          if(key[V[(opcode & 0x0F00) >> 8]] == 0)
            pc += 4;
          else
            pc += 2;
          break;
        
        default:
          goto invalid_opcode;
        
      }
      break;
    
    case 0xF000:
      switch(opcode & 0xF0FF)
      {
        case 0xF007:
          V[(opcode & 0x0F00) >> 8] = delay_timer;
          pc += 2;
          break;
          
        case 0xF00A:
          for (uint8_t i = 0; i < sizeof(key); i++) {
            if (key[i] != 0) {
                V[(opcode & 0x0F00) >> 8] = i;
                pc += 2;
            }
          }
          break;
        
        case 0xF015:
          delay_timer = V[(opcode & 0x0F00) >> 8];
          pc += 2;
          break;
          
        case 0xF018:
          sound_timer = V[(opcode & 0x0F00) >> 8];
          pc += 2;
          break;
          
        case 0xF01E:
          I += V[(opcode & 0x0F00) >> 8];
          pc += 2;
          break;

        
        case 0xF029:
          I = V[(opcode & 0x0F00) >> 8] * 5;
          pc += 2;
          break;
        
        case 0xF033:
          memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
          memory[I+1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
          memory[I+2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
          pc += 2;
          break;
        
        case 0xF055:
          for(uint8_t i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
          {
            memory[I+i] = V[i];
          }
          pc += 2;
          break;
          
        case 0xF065:
          for(uint8_t i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
          {
            V[i] = memory[I+i];
          }
          pc += 2;
          break;
        
        default:
          goto invalid_opcode;
      }
      break;
      
    default:
      goto invalid_opcode;
    }
  return 0;
invalid_opcode:
  printf("Invalid opcode %04x!\r\n", opcode);
  printf("pc: 0x%04x. sp: 0x%04x\r\n", pc, sp);
  return -1;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  uint8_t rc = 0;

  InitPlatform();
  
  LoadGame("game.ch8");

  InitCPU();
  InitGraphics();
  
  while(rc == 0)
  {
    rc = emulateCycle();
    if(draw_flag) {
      DrawGraphics();
      draw_flag = 0;
    }
    
    if(delay_timer) {
      delay_timer--;
    }
    
    if(sound_timer)
      sound_timer--;

    if(sound_timer == 1)
      PlayBeep();
    
    if(UpdateInput())
      break;
    Sleep(1);
  }
  
  printf("Oops! We fell out!\r\n");
  for(;;);
  StopGraphics();
  return 0;
}