# 👁️ Road Vision Car - Aplicação de Robótica Móvel

[![IFCE](https://img.shields.io/badge/Instituição-IFCE-green?style=for-the-badge&logo=school)](https://ifce.edu.br/)
[![ESP32](https://img.shields.io/badge/Hardware-ESP32--CAM-black?style=for-the-badge&logo=espressif)](https://www.espressif.com/)
[![C++](https://img.shields.io/badge/Language-C%2B%2B-blue?style=for-the-badge&logo=cplusplus)]()
[![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)]()

> **Processamento de imagem em tempo real na borda.**
> Firmware desenvolvido para o módulo ESP32-CAM focado em detecção de cores e reconhecimento de trajetória via análise de espectro HSV.

---

## 📋 Sobre o Projeto
Este repositório contém o firmware do sistema de visão do **Road Vision Car**. Diferente de sistemas que transmitem vídeo via Wi-Fi para processamento externo (o que gera latência), este código realiza a **análise vetorial de cores diretamente no microcontrolador**.

O sistema captura frames, converte o formato nativo RGB565 para o espaço de cores HSV e calcula a média cromática de uma Região de Interesse (ROI) central, permitindo tomadas de decisão em milissegundos.

---

## ⚙️ Como Funciona (A Lógica)

O algoritmo segue um pipeline otimizado para garantir alta taxa de quadros (FPS):

1.  **Captura Otimizada:** O sensor OV2640 é configurado em resolução `QQVGA` (160x120) para reduzir a carga de memória.
2.  **Extração de ROI:** O loop varre apenas uma **faixa vertical central** da imagem (definida por `FAIXA_LARGURA`), ignorando o restante da cena para economizar ciclos de CPU.
3.  **Bitwise Unpacking (RGB565 -> RGB888):**
    Os pixels da câmera vêm compactados em 2 bytes (5 bits vermelho, 6 verde, 5 azul). A descompactação é feita via deslocamento de bits (bit-shifting) direto na memória:
    ```cpp
    // Exemplo da lógica interna
    uint8_t r = ((pixel >> 11) & 0x1F) * 255 / 31;
    ```
4.  **Matemática de Cores (RGB -> HSV):** O código implementa uma conversão matemática manual para encontrar o *Hue* (Matiz), que é imune a variações de brilho (luz/sombra).

---

## 🔌 Hardware & Pinagem

Este firmware foi projetado para o **AI Thinker ESP32-CAM**.



| Função | Pino (GPIO) | Descrição |
| :--- | :---: | :--- |
| **D0 - D7** | Y2, Y3... | Barramento de Dados Paralelo (Câmera) |
| **XCLK** | 0 | Clock Externo |
| **PCLK** | 22 | Pixel Clock |
| **VSYNC** | 25 | Sincronia Vertical |
| **HREF** | 23 | Referência Horizontal |
| **SDA/SCL**| 26/27 | Interface I2C (SCCB) |

---

## 📊 Calibração de Cores (HSV)

O sistema detecta a cor baseada no ângulo do **Hue (0-360°)**. Abaixo está a tabela de calibração usada na função `detectaCor`:



| Cor | Intervalo Hue (°) |
| :--- | :--- |
| 🔴 **Vermelho** | `H < 15` ou `H > 330` |
| 🟠 **Laranja** | `15 ≤ H < 40` |
| 🟡 **Amarelo** | `40 ≤ H < 70` |
| 🟢 **Verde** | `70 ≤ H < 170` |
| 💠 **Ciano** | `170 ≤ H < 200` |
| 🔵 **Azul** | `200 ≤ H < 240` |
| 🟣 **Roxo** | `240 ≤ H < 310` |

> **Nota:** Se a iluminação da sala for muito fluorescente/branca, pode ser necessário ajustar o limiar do Amarelo e Laranja.

---

## 🚀 Instalação e Upload

### Pré-requisitos
1.  **Arduino IDE** com suporte a ESP32 instalado.
2.  Adaptador **BASE MB (USB-Serial)**.

### Passo a Passo
1.  Selecione a placa: **AI Thinker ESP32-CAM**.
2.  Conecte o GPIO 0 ao GND (para entrar em modo de boot).
3.  Conecte o FTDI (RX no TX, TX no RX, 5V e GND).
4.  Carregue o código.
5.  **Importante:** Remova o jumper do GPIO 0 e pressione o botão RESET na placa para iniciar.
6.  Abra o Serial Monitor em **115200 baud**.

---

## 🤝 Autores

* **Joaquim Renato de Oliveira Nogueira** - *Desenvolvimento de Firmware*
* **Emanuel Ferreira Viana** - *Documentação*
* **Luiz Isaac Pereira Sampaio** - *Integração com Embarcados*
* **Victor Almeida Marinho Rego** - *Integração com Embarcados*
* **Zion Gabriel Santos de Lima** - *Integração com Embarcados*
* **IFCE** - *Instituto Federal do Ceará*

---
<p align="center">
  <i>Desenvolvido com ☕ e C++ no Ceará.</i>
</p>
