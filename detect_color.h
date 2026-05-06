#ifndef DETECT_COLOR_H
#define DETECT_COLOR_H

#include <Arduino.h>  

// -- Armazena os parâmetros HSV de uma cor
struct hsv_t{
  int h;
  float s, v;
};

// extrai_rgb_565();
struct rgb_t {
    uint8_t r, g, b;
};

// -- Armazena o intervalo fechado dos valores HSV da cor alvo
struct color_t{
    hsv_t min_limit;
    hsv_t max_limit;
};

// -- Armazena as cores que serão identificadas
struct groupColors_t{
    color_t red;
    color_t green;
    color_t blue;
    color_t black;
};

// -- Armazena nomes de cores para que o usuário escolha qual calibrar
typedef enum {
  RED,
  GREEN,
  BLUE,
  BLACK,
} TargetColor;

// -- Detecta os valores HSV do pixel
String detectaCor(float H, float S, float V);

// -- Verifica se um pixel (h, s, v) pertence ao intervalo da cor de referência
bool isTargetColor(hsv_t pixel, color_t reference);

// -- Atribui valores em HSV para uma cor
color_t setTargetColor(hsv_t media);

#endif