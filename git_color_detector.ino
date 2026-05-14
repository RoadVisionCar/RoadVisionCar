#include "esp_camera.h"
#include "img_converters.h"
#include "detect_color.h"
#include "linha.h"

// ── Pinos AI-THINKER (sem mudança) ──────────────────────────────────
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

// ── Acumuladores multi-cor ──────────────────────────────────────────
long somaX_r = 0, somaY_r = 0; int contador_r = 0;   // Vermelho
long somaX_g = 0, somaY_g = 0; int contador_g = 0;   // Verde
long somaX_p = 0, somaY_p = 0; int contador_p = 0;   // Preto

float  cx_r = 0, cy_r = 0;   // Centro vermelho
float  cx_g = 0, cy_g = 0;   // Centro verde
float  cx_p = 0, cy_p = 0;   // Centro preto

//PercorrePixels(), rgb2hsv();
float hue, sat, val;

// CalculaCentroDeCor(), ExibirSerial();
int centro_img_x, centro_img_y;
float distx, disty;
const char *pos_x, *pos_y;

// CapturaFrame(), ConverteJPEG2RGB888
camera_fb_t *fb;

//CapturaFrame(), PercorrePixels(), CalculaCentroDeCor(), ExibeSerial(), centro_de_linha();
int largura, altura;

//PercorrePixels(), loop();
uint8_t *data;

// centro_de_linha(), rastreia_linha_preta();
int primeiro_preto, ultimo_preto;

// ── RGB → HSV ──────────────────────────────────────────
void rgb2hsv(uint8_t r, uint8_t g, uint8_t b, float &h, float &s, float &v)
{
    // Valores rgb convertidos para float
    float redNorm = r/255.0, greenNorm = g/255.0, blueNorm = b/255.0;
    // Maior e menor componente RGB
    float maxv = max(redNorm, max(greenNorm, blueNorm));
    float minv = min(redNorm, min(greenNorm, blueNorm));
    float delta = maxv - minv;
    v = maxv;

    if (delta < 0.00001) { h = 0; s = 0; return; }
    s = (maxv == 0) ? 0 : (delta / maxv);

    if (maxv == redNorm) h = 60 * fmod(((greenNorm - blueNorm) / delta), 6);
    else if (maxv == greenNorm) h = 60 * (((blueNorm - redNorm) / delta) + 2);
    else h = 60 * (((redNorm - greenNorm) / delta) + 4);

    if (h < 0) h += 360;
}

// ── Captura ──────────────────────────────────────────────────────────
void CapturaFrame(void)
{
    fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Falha ao capturar frame.");
        delay(1000);
        return;
    }
    largura = fb->width;
    altura  = fb->height;
}

// ── Percorre pixels — detecta as 3 cores simultaneamente ─────────────
void PercorrePixels(void) {
    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            int idx = (y * largura + x) * 2;        // RGB565 = 2 bytes/pixel
            data = fb->buf;

            uint16_t pixel = ((uint16_t)data[idx] << 8) | data[idx + 1];
            uint8_t r = ((pixel >> 11) & 0x1F) << 3;
            uint8_t g = ((pixel >>  5) & 0x3F) << 2;
            uint8_t b = ((pixel) & 0x1F) << 3;

            rgb2hsv(r, g, b, hue, sat, val);

            // ── Classificação do pixel ────────────────────────────────
            Color cor = detectaCor(hue, sat, val);
            bool isRed   = (cor == RED);
            bool isGreen = (cor == GREEN);
            bool isBlack = (cor == BLACK);

            // Rastreamento da linha preta (usa resultado já calculado)
            rastreia_linha_preta(detectaCor(hue, sat, val), primeiro_preto, ultimo_preto, x);

            // ── Acumulação por cor ────────────────────────────────────
            if (isRed) { somaX_r += x; somaY_r += y; contador_r++; }
            if (isGreen) { somaX_g += x; somaY_g += y; contador_g++; }
            if (isBlack) { somaX_p += x; somaY_p += y; contador_p++; }

            // ── Print amostrado (1 em cada 5 colunas) ─────────────────
            if (x % 5 == 0) {
                switch(cor)  {
                    case RED:
                        Serial.print("R ");
                        break;
                    case GREEN:
                        Serial.print("G ");
                        break;
                    case BLACK:
                        Serial.print("P ");
                        break;
                    default:
                    Serial.print("- ");
                }
            }
        }
        Serial.print("\n");
    }
}

// ── Calcula centro de uma cor e imprime ───────────────────────────────
void ExibeCor(const char* nome, long sX, long sY, int cnt, float &cx_out, float &cy_out)
{
    if (cnt == 0) {
        Serial.printf("[%s] Não detectado\n", nome);
        return;
    }
    cx_out = (float)sX / cnt;
    cy_out = (float)sY / cnt;

    float distx = cx_out - (largura  / 2);
    float disty = cy_out - (altura   / 2);

    const char* pos_x = (distx > 0) ? "direita"  : "esquerda";
    const char* pos_y = (disty > 0) ? "debaixo"  : "acima";

    Serial.printf("[%s] Centro: (%.1f, %.1f) | Dist X: %+.1f px (%s) | Dist Y: %+.1f px (%s) | Pixels: %d\n",
                  nome, cx_out, cy_out, distx, pos_x, disty, pos_y, cnt);
}

void CalculaCentroDeCor(void)
{
    centro_img_x = largura / 2;
    centro_img_y = altura  / 2;
    Serial.printf("Imagem: %d x %d | Centro: (%d, %d)\n", largura, altura, centro_img_x, centro_img_y);

    ExibeCor("VERMELHO", somaX_r, somaY_r, contador_r, cx_r, cy_r);
    ExibeCor("VERDE", somaX_g, somaY_g, contador_g, cx_g, cy_g);
    ExibeCor("PRETO", somaX_p, somaY_p, contador_p, cx_p, cy_p);
}

// ── Setup ─────────────────────────────────────────────────────────────
void setup()
{
    Serial.begin(115200);
    Serial.println("\n=== Color Tracker Multi-Cor ESP32-CAM ===");

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM; config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM; config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM; config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM; config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync  = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_RGB565;
    config.frame_size = FRAMESIZE_QQVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Erro ao iniciar câmera: 0x%x\n", err);
        return;
    }
    Serial.println("Câmera iniciada! Iniciando detecção multi-cor...\n");
}

// ── Loop ──────────────────────────────────────────────────────────────
void loop()
{
    hue = 0, sat = 0, val = 0;

    unsigned long start = millis();

    CapturaFrame();
    PercorrePixels();
    esp_camera_fb_return(fb);

    Serial.printf("Tempo de frame: %lu ms\n", millis() - start);

    CalculaCentroDeCor();
    centro_de_linha(primeiro_preto, ultimo_preto, largura);

    delay(4000);
}