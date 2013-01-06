#include <Arduino.h>
/************************
PIN MAPPING FOR iCADE PROTOCOL
Arduino    KeyCode          ASCII
Bottom right butons
8          0x0B, 0x15       h, r   BTN_A
9          0x0D, 0x11       j, n   BTN_B

Top right buttons
11          0x1C, 0x17       y, t  BTN_C
10          0x18, 0x09       u, f  BTN_D

Shoulder btns (lt/rt)
13          0x0C, 0x10       i, m  LT_SHOULDER
12          0x0E, 0x13       k, p  RT_SHOULDER

Coin/Start buttons
6         0x12, 0x0A       o, g  BTN_START(LT)
7         0x0F, 0x19       l, v  BTN_COIN(RT)

UP/DWN/LT/RT
3          0x1A, 0x08       w, e //MOVE_UP
5          0x1B, 0x1D       x, z //MOVE_DWN
4          0x04, 0x14       a, q //MOVE_LT
2          0x07, 0x06       d, c //MOVE_RT
**************************/


// This array maps a button index to an actual character to transmit
byte map_char_icade[NUM_BUTTONS][2] = {
    {0x07, 0x06},// MOVE_RT 2
    {0x1A, 0x08},// MOVE_UP 3
    {0x04, 0x14},// MOVE_LT 4
    {0x1B, 0x1D},// MOVE_DWN 5
    {0x12, 0x0A},// BTN_START 6
    {0x0F, 0x19},// BTN_COIN 7
    {0x0B, 0x15},// BTN_A 8
    {0x0D, 0x11},// BTN_B 9
    {0x18, 0x09},// BTN_D 10
    {0x1C, 0x17},// BTN_C 11
    {0x0E, 0x13},// RT_SHOULDER 12
    {0x0C, 0x10}};// LT_SHOULDER 13

// This is the current and previous state of each button 0 up, 1 down
// each pair is the current and previous state
byte btn_state_icade[NUM_BUTTONS][2] = {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
  {0,0},{0,0},{0,0},{0,0}};
