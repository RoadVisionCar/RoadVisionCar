#include "detect_color.h"


// ===== CALCULA O CENTRO DA LINHA =====
void centro_de_linha (int primeiro_preto, int ultimo_preto, int largura){
  if (primeiro_preto != -1) {
    int centro_linha          = (primeiro_preto + ultimo_preto) / 2;
    int centro_imagem         = largura / 2;  // = 80 para 160px de largura
    int desvio_centro_linha   = centro_linha - centro_imagem;

    // ===== DECIDE DIREÇÃO =====
    // desvio_centro_linha negativo = linha à esquerda
    // desvio_centro_linha zero     = linha no centro
    // desvio_centro_linha positivo = linha à direita
    if (desvio_centro_linha < -15) {
      Serial.println("⬅ VIRAR ESQUERDA   | desvio_centro_linha: " + String(desvio_centro_linha));
    } else if (desvio_centro_linha > 15) {
      Serial.println("➡ VIRAR DIREITA    | desvio_centro_linha: " + String(desvio_centro_linha));
    } else {
      Serial.println("⬆ SEGUIR EM FRENTE | desvio_centro_linha: " + String(desvio_centro_linha));
    }

    Serial.print("   Linha: pixel ");
    Serial.print(primeiro_preto);
    Serial.print(" até ");
    Serial.print(ultimo_preto);
    Serial.print(" | Centro da linha: ");
    Serial.println(centro_linha);

  } else {
    Serial.println("\n⚠ LINHA NÃO ENCONTRADA — parar ou buscar");
  }
}

int rastreia_linha_preta (String cor_HSV, int primeiro_preto, int ultimo_preto, int x){
  if (cor_HSV == "Preto") {
        if (primeiro_preto == -1) {
          return primeiro_preto = x;
        }
        return ultimo_preto = x;
      }
}