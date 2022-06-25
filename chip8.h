#ifndef CHIP8_H
#define CHIP8_H

#define SCREEN_WIDTH (64)
#define SCREEN_HEIGHT (32)
#define GAME_SIZE (3584)

extern uint16_t opcode;

extern uint8_t memory[4096];
extern uint8_t V[16];
extern uint16_t I;
extern uint16_t pc;

extern uint8_t gfx[SCREEN_WIDTH][SCREEN_HEIGHT];
extern uint8_t gfx_small[8][SCREEN_HEIGHT];
extern uint8_t delay_timer;
extern uint8_t sound_timer;

extern uint16_t stack[16];
extern uint16_t sp;

extern uint8_t key[16];

extern uint16_t draw_flag;

extern uint8_t chip8_fontset[80];

uint8_t InitPlatform(void);

uint8_t LoadGame(char* filename);

uint8_t InitGraphics(void);
uint8_t DrawGraphics(void);
uint8_t StopGraphics(void);

uint8_t UpdateInput(void);

uint8_t PlayBeep(void);

#endif /* CHIP8_H */