#include "detect_color.h"

String detectaCor(float H, float S, float V)
{
  bool sat_value = (S>= 0.20 || V>= 0.10);

  if (V < 0.25)               return "Preto";
  if (V > 0.85 && S < 0.15)   return "Branco";

  if (sat_value)
  {
    if (H < 30 || H > 330)    return "Vermelho";
    if (H > 90 && H < 170)    return "Verde";
    if (H > 180 && H < 260)   return "Azul";
  }
  return "INDEFINIDO";
}


// -- Verifica se um pixel (h, s, v) pertence ao intervalo da cor de referência
bool isTargetColor(hsv_t pixel, color_t reference)
{
    if ((pixel.h > reference.min_limit.h && pixel.h < reference.max_limit.h) &&
        (pixel.s > reference.min_limit.s && pixel.s < reference.max_limit.s) &&
        (pixel.v > reference.min_limit.v && pixel.v < reference.max_limit.v)) {
            return true;
        }

    return false;
}


// -- Atribui valores em HSV para uma cor
color_t setTargetColor(hsv_t media)
{
    // margens de erro para definir o intervalo identificável da cor alvo
    hsv_t desvio = {

        // a definir
        .h = 20,
        .s = 0.1, 
        .v = 0.1, 
    };

    // limites máximos do intervalo da cor alvo
    hsv_t max_hsv = {
        .h = media.h + desvio.h,
        .s = media.s + desvio.s,
        .v = media.v + desvio.v,
    };

    // limites mínimos do intervalo da cor alvo
    hsv_t min_hsv = {
        .h = media.h - desvio.h,
        .s = media.s - desvio.s,
        .v = media.v - desvio.v,
    };
    
    // contém o intervalo fechado dos valores HSV da cor alvo
    color_t target_color = {
        .min_limit = min_hsv,
        .max_limit = max_hsv,
    };

    return target_color;
}