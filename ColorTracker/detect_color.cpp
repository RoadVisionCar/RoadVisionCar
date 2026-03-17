#include "detect_color.h"

/*
 * Verifica se o pixel (r, g, b) corresponde à cor alvo.
 *
 * Os limiares abaixo foram mantidos iguais ao código original.
 * Ajuste-os se a iluminação do ambiente afetar a detecção.
 */
int isTargetColor(int r, int g, int b, Color target)
{
    switch (target)
    {
        case RED:
            return (r > 200 && g < 80 && b < 80);

        case BLUE:
            return (b > 200 && r < 120 && g < 180);

        case GREEN:
            return (g > 160 && r < 80 && b < 100);
    }

    return 0;
}
