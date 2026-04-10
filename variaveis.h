#ifndef VARIAVEIS_H
#define VARIAVEIS_H

//PercorrePixels(), rgb2hsv();
float H, S, V;

// CapturaFrame(), ConverteJPEG2RGB888
camera_fb_t *fb;

//CapturaFrame(), PercorrePixels(), CalculaCentroDeCor(), ExibeSerial(), centro_de_linha();
int largura;

//CapturaFrame(), PercorrePixels(), CalculaCentroDeCor(), ExibeSerial()
int altura;

//PercorrePixels(), loop();
//uint8_t *rgb_buf;
uint8_t *data;

// PercorrePixels(), CalculaCentroDeCor();
long somaX, somaY;

// PercorrePixels(), CalculaCentroDeCor(), ExibirSerial();
int contador;

// centro_de_linha(), rastreia_linha_preta();
int primeiro_preto, ultimo_preto;

// CalculaCentroDeCor(), ExibirSerial();
int centro_img_x, centro_img_y;
float cx, cy, distx, disty;
const char *pos_x, *pos_y;

#endif