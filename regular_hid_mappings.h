/************************
PIN MAPPING REGULAR PROTOCOL
Arduino    KeyCode    ASCII
8          0x04       a  BTN_A
9          0x05       b  BTN_B

11         0x06       c  BTN_C
10         0x07       d  BTN_D
    
13         0x08       e  LT_SHOULDER
12         0x09       f  RT_SHOULDER

3          0x52       UP ARROW
5          0x51       DOWN ARROW
4          0x50       LEFT ARROW
2          0x4F       RIGHT ARROW

6          0x10       m  BTN_START(LT)
7          0x13       p  BTN_COIN(RT)
**************************/
#include <Arduino.h>

// This array maps a button index to an actual character to transmit
byte map_char[NUM_BUTTONS] = {
    0x4F, //RIGHT ARROW 2
    0x52, //UP ARROW 3
    0x50, //LEFT ARROW 4
    0x51, //DOWN ARROW 5
    0x10, //BTN_START 6
    0x13, //BTN_COIN 7
    0x04, //BTN_A 8
    0x05, //BTN_B 9
    0x07, //BTN_D 10
    0x06, //BTN_C 11
    0x09, //RT_SHOULDER 12
    0x08}; //LT_SHOULDER 13
