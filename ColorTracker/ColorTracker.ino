/*
 * ============================================================
 *  Color Tracker — ESP32-CAM
 *  Detecta a cor alvo em cada frame e calcula o centroide
 * ============================================================
 *
 *  Pinout padrão para o módulo AI-THINKER ESP32-CAM.
 *  Se usar outro módulo, ajuste os pinos abaixo.
 */

#include "esp_camera.h"
#include "img_converters.h"
#include "detect_color.h"

// ── Escolha a cor alvo aqui ──────────────────────────────────
// Opções: RED, GREEN, BLUE
#define COR_ALVO BLUE
// ────────────────────────────────────────────────────────────

// ── Pinos AI-THINKER ────────────────────────────────────────
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
// ────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Color Tracker ESP32-CAM ===");

    // ── Configuração da câmera ───────────────────────────────
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;
    config.pin_d0       = Y2_GPIO_NUM;
    config.pin_d1       = Y3_GPIO_NUM;
    config.pin_d2       = Y4_GPIO_NUM;
    config.pin_d3       = Y5_GPIO_NUM;
    config.pin_d4       = Y6_GPIO_NUM;
    config.pin_d5       = Y7_GPIO_NUM;
    config.pin_d6       = Y8_GPIO_NUM;
    config.pin_d7       = Y9_GPIO_NUM;
    config.pin_xclk     = XCLK_GPIO_NUM;
    config.pin_pclk     = PCLK_GPIO_NUM;
    config.pin_vsync    = VSYNC_GPIO_NUM;
    config.pin_href     = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn     = PWDN_GPIO_NUM;
    config.pin_reset    = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // Resolução menor = mais rápido e menos RAM usada
    // Opções: FRAMESIZE_QQVGA (160x120), FRAMESIZE_QVGA (320x240)
    config.frame_size   = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;   // 0–63 (menor = melhor qualidade)
    config.fb_count     = 1;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Erro ao iniciar câmera: 0x%x\n", err);
        return;
    }

    Serial.println("Câmera iniciada com sucesso!");
    Serial.println("Iniciando detecção...\n");
}
// ================== RGB → HSV ==================
void rgb2hsv(uint8_t r, uint8_t g, uint8_t b,
             float &h, float &s, float &v) {

  float rf = r / 255.0;
  float gf = g / 255.0;
  float bf = b / 255.0;

  float maxv = max(rf, max(gf, bf));
  float minv = min(rf, min(gf, bf));
  float delta = maxv - minv;

  v = maxv;

  if (delta < 0.00001) {
    h = 0;
    s = 0;
    return;
  }

  s = (maxv == 0) ? 0 : (delta / maxv);

  if (maxv == rf)
    h = 60 * fmod(((gf - bf) / delta), 6);
  else if (maxv == gf)
    h = 60 * (((bf - rf) / delta) + 2);
  else
    h = 60 * (((rf - gf) / delta) + 4);

  if (h < 0) h += 360;
}
// ================== DETECÇÃO DE COR ==================
String detectaCor(float H, float S, float V) {

  if (V < 0.15) return "Preto";
  if (V > 0.85 && S < 0.20) return "Branco";

  if (H < 15 || H > 330) return "Vermelho";
  if (H < 40)            return "Laranja";
  if (H < 70)            return "Amarelo";
  if (H < 170)           return "Verde";
  if (H < 200)           return "Ciano";
  if (H < 240)           return "Azul";
  if (H < 310)           return "Roxo";

  return "Indefinido";
}

void loop() {
    // ── 1. Captura o frame ───────────────────────────────────
    camera_fb_t *fb = esp_camera_fb_get();   // SDK 2.x
    if (!fb) {
        Serial.println("Falha ao capturar frame.");
        delay(1000);
        return;
    }

    int largura = fb->width;
    int altura  = fb->height;

    // ── 2. Converte JPEG → RGB888 ────────────────────────────
    // fmt2rgb888 no SDK 2.x recebe uint8_t* (não ponteiro duplo)
    // então alocamos o buffer manualmente antes de chamar
    size_t   rgb_len = largura * altura * 3;
    uint8_t *rgb_buf = (uint8_t *)malloc(rgb_len);

    if (rgb_buf == NULL) {
        Serial.println("Sem memoria para o buffer RGB.");
        esp_camera_fb_return(fb);
        delay(500);
        return;
    }

    bool ok = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, rgb_buf);
    esp_camera_fb_return(fb);   // libera o frame o quanto antes — SDK 2.x

    if (!ok) {
        Serial.println("Falha ao converter imagem para RGB.");
        free(rgb_buf);
        delay(500);
        return;
    }

    // ── 3. Percorre os pixels (mesma lógica do código original) ─
    long somaX    = 0;
    long somaY    = 0;
    int  contador = 0;

    
    int primeiro_preto = -1;  // -1 = ainda não encontrou nenhum pixel preto
    int ultimo_preto   = -1;

    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            

            // Variáveis para calcular a cor média da faixa (útil para linhas coloridas)
            float somaH = 0;
            float somaS = 0;
            float somaV = 0;
            int cont = 0;
            int idx = (y * largura + x) * 3;  // RGB888 = 3 bytes por pixel

            int r = rgb_buf[idx];
            int g = rgb_buf[idx + 1];
            int b = rgb_buf[idx + 2];

            float H, S, V;
      rgb2hsv(r, g, b, H, S, V);

      // Rastreia a posição da linha preta
      if (detectaCor(H, S, V) == "Preto") {
        if (primeiro_preto == -1) {
          primeiro_preto = x;
        }
        ultimo_preto = x;
      }

      // Acumula os valores HSV de todos os pixels da faixa
      somaH += H;
      somaS += S;
      somaV += V;
      cont++;

            if (isTargetColor(r, g, b, COR_ALVO)) {
                somaX += x;
                somaY += y;
                contador++;
            }
        }
    }

    free(rgb_buf);  // libera o buffer RGB

    // ── 4. Calcula e exibe o resultado ───────────────────────
    int centro_img_x = largura / 2;
    int centro_img_y = altura  / 2;

    Serial.printf("Imagem: %d x %d | Centro: (%d, %d)\n",
                  largura, altura, centro_img_x, centro_img_y);

    if (contador > 0) {

        float cx = (float)somaX / contador;
        float cy = (float)somaY / contador;

        float distx = cx - centro_img_x;
        float disty = cy - centro_img_y;

        const char *pos_x = (cx > centro_img_x) ? "direita"   : "esquerda";
        const char *pos_y = (cy > centro_img_y) ? "debaixo"   : "acima";

        Serial.printf("DETECTADO — Centro da cor: (%.1f, %.1f)\n", cx, cy);
        Serial.printf("  Dist X: %+.1f px (%s)\n", distx, pos_x);
        Serial.printf("  Dist Y: %+.1f px (%s)\n", disty, pos_y);
        Serial.printf("  Pixels detectados: %d\n\n", contador);

    } else {
        Serial.println("Cor nao encontrada no frame.\n");
    }
    // ===== CALCULA O CENTRO DA LINHA =====
  if (primeiro_preto != -1) {
    int centro_linha  = (primeiro_preto + ultimo_preto) / 2;
    int centro_imagem = largura / 2;  // = 80 para 160px de largura
    int erro          = centro_linha - centro_imagem;

    // ===== DECIDE DIREÇÃO =====
    // erro negativo = linha à esquerda
    // erro zero     = linha no centro
    // erro positivo = linha à direita
    if (erro < -15) {
      Serial.println("⬅ VIRAR ESQUERDA   | erro: " + String(erro));
    } else if (erro > 15) {
      Serial.println("➡ VIRAR DIREITA    | erro: " + String(erro));
    } else {
      Serial.println("⬆ SEGUIR EM FRENTE | erro: " + String(erro));
    }

    Serial.print("   Linha: pixel ");
    Serial.print(primeiro_preto);
    Serial.print(" até ");
    Serial.print(ultimo_preto);
    Serial.print(" | Centro da linha: ");
    Serial.println(centro_linha);

  } else {
    Serial.println("⚠ LINHA NÃO ENCONTRADA — parar ou buscar");
  }

  delay(500);
    //delay(300);  // ~3 frames por segundo
}
