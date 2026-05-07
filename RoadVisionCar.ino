#include "esp_camera.h"
#include "img_converters.h"
#include "detect_color.h"
#include "linha.h"
#include "variaveis.h"

// Escolher a cor a ser calibrada (RED, GREEN, BLUE, BLACK)
#define COR_ALVO RED

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

// -- Calibra o ESP32 CAM para associar as cores identificadas às cores de referência
void calibrar()
{
    hsv_t media_hsv;

    // passo de extração da cor do pixel da imagem capturada
    int passo = 20, qnt_pixels = ((int) (altura/passo)) * ((int) (largura/passo));
    float soma_h = 0, soma_s = 0, soma_v = 0;
    float media_h = 0, media_s = 0, media_v = 0;

    // obtém a imagem 240 x 320 px
    CapturaFrame();
 
    // varredura dos pixels da imagem em função do passo
    for (int y = 0; y < altura; y+= passo)
    {
        for (int x = 0; x < largura; x+= passo)
        {
            rgb_t cor = Extrai_rgb_565(x, y);

            rgb2hsv(cor, H, S, V);

            soma_h += H;
            soma_s += S;
            soma_v += V;
        }
    }

    media_hsv = {

        // casting reforçando o tipo de dado de hue, saturation e value
        .h =  (int) soma_h/qnt_pixels,
        .s =  (float) soma_s/qnt_pixels,
        .v =  (float) soma_v/qnt_pixels,
    };

    // calibração de uma cor de groupColors_t mediante à escolha da COR_ALVO
    switch (COR_ALVO) {
        case RED:
            referenceGroup.red = setTargetColor(media_hsv);
            break;
        case BLUE:
            referenceGroup.blue = setTargetColor(media_hsv);
            break;
        case GREEN:
            referenceGroup.green = setTargetColor(media_hsv);
            break;
        case BLACK:
            referenceGroup.black = setTargetColor(media_hsv);
            break;
    }
}


// RGB to HSV 
void rgb2hsv(rgb_t pixel, float &h, float &s, float &v)
{

    float rf = pixel.r / 255.0;
    float gf = pixel.g / 255.0;
    float bf = pixel.b / 255.0;

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


void CapturaFrame(void)
{
    fb = esp_camera_fb_get();   // SDK 2.x
    
    if (!fb)
    {
        Serial.println("Falha ao capturar frame.");
        delay(1000);
        return;
    }

    largura = fb->width; //240px
    altura  = fb->height; //320px
}


// tipo de dado: RGB; ver em variaveis.h
rgb_t Extrai_rgb_565(int x, int y)
{
    // idx: index (posicao na memoria)
    int idx = (y * largura + x) * 2;  // RGB565 = 2 bytes por pixel

    // data: dados de cada pixel da captura de imagem
    data = fb->buf;

    // Reconstroi o uint16_t a partir dos 2 bytes
    uint16_t pixel = ((uint16_t) data[idx] << 8) | data[idx + 1];

    // Extrai e expande cada canal para 8 bits
    return {
        .r = (uint8_t)(((pixel >> 11) & 0x1F) << 3),
        .g = (uint8_t)(((pixel >>  5) & 0x3F) << 2),
        .b = (uint8_t)(((pixel      ) & 0x1F) << 3)
    };
}


void PercorrePixels(void)
{
    // cor usada de referência
    color_t referenceColor;
    hsv_t detected_hsv;

    // percorre todos os pixels da imagem
    for (int y = 0; y < altura; y++)
    {
        for (int x = 0; x < largura; x++)
        {
            rgb_t cor = Extrai_rgb_565(x, y);

            rgb2hsv(cor, H, S, V);

            detected_hsv = {
                .h = H,
                .s = S, 
                .v = V,
            };

            // rastreia a posição da linha preta
            rastreia_linha_preta (detectaCor(H, S, V), primeiro_preto, ultimo_preto, x);

            switch (COR_ALVO) {
                case RED:
                    referenceColor = referenceGroup.red;
                    break;
                case BLUE:
                    referenceColor = referenceGroup.blue;
                    break;
                case GREEN:
                    referenceColor = referenceGroup.green;
                    break;
                case BLACK:
                    referenceColor = referenceGroup.black;
                    break;
            }

            // a cada 5 pixels da largura da imagem, verifica se o pixel contém a COR_ALVO
            if ((x%5 == 0) && isTargetColor(detected_hsv, referenceColor))
            {
                somaX += x;
                somaY += y;
                contador++;

                Serial.print("1 ");
            }
            else if(x%5 == 0)
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
    config.pixel_format = PIXFORMAT_RGB565;          
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

    calibrar();

   // inicia contagem de tempo 
    unsigned long start = millis();

    // 1. Captura o frame CRIAR FUNÇÃO À PARTE
    CapturaFrame();

    // 2. Percorre os pixels (mesma lógica do código original)
    PercorrePixels();    

    esp_camera_fb_return(fb);  // libera o buffer da captura da imagem

    // tempo decorrido até agora
    unsigned long elapsed = millis() - start;
    Serial.printf("Tempo decorrido: %i ms\n", elapsed);

    // 3. Calcula e exibe o resultado
    CalculaCentroDeCor();

    // 4. CALCULA O CENTRO DA LINHA
    centro_de_linha (primeiro_preto, ultimo_preto, largura);

    delay(4000);
}