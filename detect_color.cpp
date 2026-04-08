#include "detect_color.h"

String detectaCor(float H, float S, float V)
{
  int sat_value = (S >= 0.20 || V >= 0.25);

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
/*
 * Verifica se o pixel (h, s, v) corresponde à cor alvo.
 * Os limiares abaixo foram mantidos iguais ao código original.
 * Ajuste-os se a iluminação do ambiente afetar a detecção.
 */
int isTargetColor(float h, float s, float v, Color target){
    int sat_value = (s < 0.20 || v < 0.15);
    //if (sat_value) return 0; // muito cinza ou escuro
    switch (target)
    {
        // Intervalo para identificacao de vermelho aumentado (algoritimo simplista)
        case RED:
            return ((h < 30 || h > 330) && s > 0.20 && v > 0.25);
        case BLUE:
            return ((h > 180 && h < 260) && s > 0.20 && v > 0.25);
        case GREEN:
            return ((h > 90 && h < 170) && s > 0.20 && v > 0.25);
        case BLACK:
            return (v < 0.25);
    }

    return 0;
}
