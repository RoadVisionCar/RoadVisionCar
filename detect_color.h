#ifndef DETECT_COLOR_H
#define DETECT_COLOR_H

#include <Arduino.h>  

// Enum para representar as cores suportadas
typedef enum {
    RED,
    GREEN,
    BLUE,
    BLACK,
    Indefinido
} Color;

// Detecta os valores HSV do pixel
Color detectaCor(float h, float s, float v);

// Verifica se um pixel (r, g, b) pertence à cor alvo
int isTargetColor(float h, float s, float v, Color target);

#endif