#ifndef DETECT_COLOR_H
#define DETECT_COLOR_H

#include <Arduino.h>   // necessário no ambiente Arduino

// Enum para representar as cores suportadas
typedef enum {
    RED,
    GREEN,
    BLUE
} Color;

// Detecta os valores HSV do pixel
String detectaCor(float H, float S, float V);

// Verifica se um pixel (r, g, b) pertence à cor alvo
int isTargetColor(float h, float s, float v, Color target);

#endif