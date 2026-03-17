#ifndef DETECT_COLOR_H
#define DETECT_COLOR_H

#include <Arduino.h>   // necessário no ambiente Arduino

// Enum para representar as cores suportadas
typedef enum {
    RED,
    GREEN,
    BLUE
} Color;

// Verifica se um pixel (r, g, b) pertence à cor alvo
int isTargetColor(int r, int g, int b, Color target);

#endif
