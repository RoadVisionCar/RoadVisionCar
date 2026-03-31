#ifndef LINHA_H
#define LINHA_H

#include <Arduino.h>  

// Calcula o centro da linha preta
void centro_de_linha (int primeiro_preto, int ultimo_preto, int largura);

// Rastreia as bordas da linha preta
int rastreia_linha_preta (String cor_HSV, int primeiro_preto, int ultimo_preto, int x);

#endif