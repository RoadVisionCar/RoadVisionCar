#include "esp_camera.h"
#include "img_converters.h"
#include "detect_color.h"
#include "linha.h"
#include "variaveis.h"

// Escolher a cor do alvo (RED, GREEN, BLUE, BLACK)
#define COR_ALVO BLACK

// Definição de pinos AI-THINKER 
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

// RGB → HSV 
void rgb2hsv(uint8_t r, uint8_t g, uint8_t b, float &h, float &s, float &v)
{

    float rf = r / 255.0;
    float gf = g / 255.0;
    float bf = b / 255.0;

    float maxv = max(rf, max(gf, bf));
    float minv = min(rf, min(gf, bf));
    float delta = maxv - minv;

    v = maxv;

    if (delta < 0.00001)
    {
        h = 0;
        s = 0;
        return;
    }

    if (maxv == 0)
    {
        s = 0;
    }
    else
    {
        s = (delta / maxv);
    }

    if (maxv == rf)
    {
        h = 60 * fmod(((gf - bf) / delta), 6);
    }
    else if (maxv == gf)
    {
        h = 60 * (((bf - rf) / delta) + 2);
    }
    else
    {
        h = 60 * (((rf - gf) / delta) + 4);
    }

    if (h < 0)
    {
        h += 360;
    }
}

void CapturaFrame (void)
{
    /*
    typedef struct {
        uint8_t * buf;              /*!< Pointer to the pixel data 
      size_t len;                 /*!< Length of the buffer in bytes 
      size_t width;               /*!< Width of the buffer in pixels 
      size_t height;              /*!< Height of the buffer in pixels 
      pixformat_t format;         !< Format of the pixel data 
      struct timeval timestamp;   !< Timestamp since boot of the first DMA buffer of the frame 
      } camera_fb_t;
    */

    fb = esp_camera_fb_get();   // SDK 2.x
    if (!fb)
    {
        Serial.println("Falha ao capturar frame.");
        delay(1000);
        return;
    }

    largura = fb->width;
    altura  = fb->height;
}

void ConverteJPEG2RGB888(void)
{
    // fmt2rgb888 no SDK 2.x recebe uint8_t* (não ponteiro duplo)
    // então alocamos o buffer manualmente antes de chamar

    size_t   rgb_len = largura * altura * 3;
    rgb_buf = (uint8_t *)malloc(rgb_len);

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
}

void PercorrePixels(void)
{
    somaX    = 0;
    somaY    = 0;
    contador = 0;

    // primeiro pixel preto a ser detectado
    primeiro_preto = -1;  // -1 = ainda não encontrou nenhum pixel preto

    // ultimo pixel preto a ser detectado
    ultimo_preto   = -1;

    for (int y = 0; y < altura; y++)
    {
        for (int x = 0; x < largura; x++)
        {
            // idx: index (posicao na memoria)
            int idx = (y * largura + x) * 3;  // RGB888 = 3 bytes por pixel

            int r = rgb_buf[idx];
            int g = rgb_buf[idx + 1];
            int b = rgb_buf[idx + 2];

            rgb2hsv(r, g, b, H, S, V);

      // Rastreia a posição da linha preta
      rastreia_linha_preta (detectaCor(H, S, V), primeiro_preto, ultimo_preto, x);

            if ((x%5==0)&&(isTargetColor(H, S, V, COR_ALVO)))
            {
                somaX += x;
                somaY += y;
                contador++;
                Serial.print("1 ");
            }
            else if(x%5==0)
            {
              Serial.print("- ");
            }
      }
      Serial.print("\n");
    }
}

void CalculaCentroDeCor(void)
{
    centro_img_x = largura / 2;
    centro_img_y = altura  / 2;

    if (contador > 0)
    {

        cx = (float)somaX / contador;
        cy = (float)somaY / contador;

        distx = cx - centro_img_x;
        disty = cy - centro_img_y;

        pos_x = (distx > 0) ? "direita"   : "esquerda";
        pos_y = (disty > 0) ? "debaixo"   : "acima";

        ExibeSerial();
    }
    else
    {
        Serial.print("Cor alvo nao encontrada no frame.\n");
        /*
        Serial.printf("'%s' (HSV = %.1f, %.1f, %.1f)\n", detectaCor(H, S, V), H, S, V);
        */
    }
}

void ExibeSerial(void)
{
    Serial.printf("Imagem: %d x %d | Centro: (%d, %d)\n", largura, altura, centro_img_x, centro_img_y);
    Serial.printf("DETECTADO — Centro da cor: (%.1f, %.1f)\n", cx, cy);
    Serial.printf("  Dist X: %+.1f px (%s)\n", distx, pos_x);
    Serial.printf("  Dist Y: %+.1f px (%s)\n", disty, pos_y);
    Serial.printf("  Pixels detectados: %d\n\n", contador);
}

void setup()
{
    // Definição de Baud Rate 
    Serial.begin(115200);
    Serial.println("\n=== Color Tracker ESP32-CAM ===");

    // Configuração da câmera 
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
    config.pixel_format = PIXFORMAT_JPEG;           //trocar por rgb565 (melhor identificação de cores)
    config.frame_size   = FRAMESIZE_QQVGA;
    config.jpeg_quality = 12;                       // 0–63 (menor = melhor qualidade)
    config.fb_count     = 1;                        // apenas 1 buffer para receber a imagem por vez

    // inicializa a camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Erro ao iniciar câmera: 0x%x\n", err);
        return;
    }

    Serial.println("Câmera iniciada com sucesso!");
    Serial.println("Iniciando detecção...\n");
}

void loop()
{
    H = 0, S = 0, V = 0;

    // 1. Captura o frame CRIAR FUNÇÃO À PARTE
    CapturaFrame();

    // 2. Converte JPEG → RGB888 
    ConverteJPEG2RGB888();

    // 3. Percorre os pixels (mesma lógica do código original)
    PercorrePixels();    

    free(rgb_buf);  // libera o buffer RGB

    // 4. Calcula e exibe o resultado
    CalculaCentroDeCor();

    // 5. CALCULA O CENTRO DA LINHA
    centro_de_linha (primeiro_preto, ultimo_preto, largura);

    delay(4000);
}