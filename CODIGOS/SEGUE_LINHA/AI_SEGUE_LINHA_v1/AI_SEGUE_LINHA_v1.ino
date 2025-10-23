// ===============================================================
// 🔵 PROJETO: AI CHALLENGER - SEGUE LINHA
// 🤖 Desafio Azoresbot – Versão com HuskyLens + OLED SH1106 + LEDs
// 🧠 Controle proporcional com avanço/pausa para estabilidade
// 📡 Comunicação UART com a HuskyLens (SoftwareSerial)
// 📺 Visualização de estado e diagnóstico no display OLED I2C
// 💡 LEDs indicativos: Verde (Frente), Azul (Direita), Amarelo (Esquerda), Vermelho (Parado, piscando)
// ===============================================================

#include <SoftwareSerial.h>
#include "HUSKYLENS.h"
#include <Wire.h>
#include <U8x8lib.h>

// === Comunicação com HuskyLens via UART ===
// Pino 9 (TX Arduino) → RX da HuskyLens → fio azul
// Pino 8 (RX Arduino) → TX da HuskyLens → fio verde
SoftwareSerial huskySerial(8, 9); // RX: 8, TX: 9
HUSKYLENS huskylens;

// === OLED SH1106 ===
U8X8_SH1106_128X64_NONAME_HW_I2C oled(U8X8_PIN_NONE);

// === Pinos do driver L298P ===
const int E1 = 10;  // PWM direita (controle de velocidade do motor direito)
const int M1 = 12;  // DIR direita (direção do motor direito)
const int E2 = 11;  // PWM esquerda (controle de velocidade do motor esquerdo)
const int M2 = 13;  // DIR esquerda (direção do motor esquerdo)

// === Pinos dos LEDs indicativos ===
const int ledVerde = 4;    // LED verde: indica movimento para frente (reto)
const int ledAzul = 5;     // LED azul: indica correção para a direita
const int ledAmarelo = 6;  // LED amarelo: indica correção para a esquerda
const int ledVermelho = 7; // LED vermelho: pisca quando o carro está parado

// === Parâmetros de controle ===
const float Kp = 0.5;              // Ganho proporcional (ajustar conforme necessário)
const int velocidade_base = 120;   // Velocidade base dos motores (0-255)
const int advanceTime = 120;       // Tempo de avanço do carro (ms)
const int stopTime = 150;          // Tempo de parada para estabilização (ms)
const int centerLine = 160;        // Centro da imagem da HuskyLens (320x240)
const int tolerance = 20;          // Tolerância para considerar a linha centrada (pixels)

// === Controle do piscar do LED vermelho ===
unsigned long previousMillis = 0;   // Último tempo de alternância do LED
const long blinkInterval = 500;    // Intervalo de piscar do LED vermelho (ms)
bool ledVermelhoState = false;     // Estado atual do LED vermelho (ligado/desligado)

void setup() {
  // Inicializa comunicação serial para depuração
  Serial.begin(9600);

  // Inicializa comunicação UART com a HuskyLens
  huskySerial.begin(9600);
  huskylens.begin(huskySerial);
  huskylens.writeAlgorithm(ALGORITHM_LINE_TRACKING); // Define modo de rastreamento de linha

  // Inicializa display OLED
  oled.begin();
  oled.setFont(u8x8_font_chroma48medium8_r); // Fonte legível para o SH1106
  oled.drawString(0, 0, "AzorCAR Iniciado"); // Mensagem inicial

  // Configura pinos do driver L298P
  pinMode(E1, OUTPUT); pinMode(M1, OUTPUT);
  pinMode(E2, OUTPUT); pinMode(M2, OUTPUT);

  // Configura pinos dos LEDs
  pinMode(ledVerde, OUTPUT);
  pinMode(ledAzul, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVermelho, OUTPUT);

  // Para os motores e apaga LEDs inicialmente
  parar();
  apagarLEDs();
}

void loop() {
  // Solicita dados da HuskyLens
  if (!huskylens.request()) {
    oled.clear();
    oled.drawString(0, 0, "Erro HuskyLens"); // Indica falha na comunicação
    parar();
    piscarLEDVermelho(); // Pisca LED vermelho
    delay(stopTime);
    return;
  }

  // Lê resultado da HuskyLens
  HUSKYLENSResult result = huskylens.read();
  if (result.command == COMMAND_RETURN_ARROW) {
    // Calcula erro: diferença entre a posição da linha e o centro da imagem
    int erro = result.xTarget - centerLine;

    // Controle proporcional: calcula correção com base no erro
    float correcao = Kp * erro;
    int vel_esq = constrain(velocidade_base + correcao, 0, 255); // Velocidade motor esquerdo
    int vel_dir = constrain(velocidade_base - correcao, 0, 255); // Velocidade motor direito

    // Atualiza display OLED com informações
    oled.clear();
    oled.drawString(0, 0, "Linha OK"); // Linha detectada
    oled.setCursor(0, 1);
    oled.print("Erro: ");
    oled.print(erro); // Exibe erro
    oled.setCursor(0, 2);
    oled.print("Esq:");
    oled.print(vel_esq);
    oled.print(" Dir:");
    oled.print(vel_dir); // Exibe velocidades dos motores

    // Verifica se a linha está centrada (dentro da tolerância)
    if (abs(erro) < tolerance) {
      andarFrente(velocidade_base, velocidade_base); // Segue reto
      oled.drawString(0, 3, "Reto");
      apagarLEDs();
      digitalWrite(ledVerde, HIGH); // Acende LED verde (frente)
    } else {
      andarFrente(vel_esq, vel_dir); // Corrige direção
      oled.drawString(0, 3, "Corrige");
      apagarLEDs();
      if (erro > 0) {
        digitalWrite(ledAzul, HIGH); // Linha à direita, acende LED azul
      } else {
        digitalWrite(ledAmarelo, HIGH); // Linha à esquerda, acende LED amarelo
      }
    }

    // Avança por advanceTime ms
    delay(advanceTime);

    // Para para estabilizar e ler novamente
    parar();
    oled.clearLine(3);
    oled.drawString(0, 3, "Parado");
    apagarLEDs();
    piscarLEDVermelho(); // Pisca LED vermelho
    delay(stopTime);
  } else {
    oled.clear();
    oled.drawString(0, 0, "Sem Linha"); // Nenhuma linha detectada
    parar();
    piscarLEDVermelho(); // Pisca LED vermelho
    delay(stopTime);
  }
}

// Função para mover o carro para frente com velocidades especificadas
void andarFrente(int vel_esq, int vel_dir) {
  digitalWrite(M1, LOW);  // Motor direito: direção frente
  digitalWrite(M2, LOW);  // Motor esquerdo: direção frente
  analogWrite(E1, vel_dir); // Aplica PWM ao motor direito
  analogWrite(E2, vel_esq); // Aplica PWM ao motor esquerdo
}

// Função para parar os motores
void parar() {
  analogWrite(E1, 0); // Desliga motor direito
  analogWrite(E2, 0); // Desliga motor esquerdo
}

// Função para apagar todos os LEDs
void apagarLEDs() {
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledAzul, LOW);
  digitalWrite(ledAmarelo, LOW);
  digitalWrite(ledVermelho, LOW);
}

// Função para piscar o LED vermelho (não bloqueante)
void piscarLEDVermelho() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= blinkInterval) {
    ledVermelhoState = !ledVermelhoState; // Alterna estado
    digitalWrite(ledVermelho, ledVermelhoState ? HIGH : LOW);
    previousMillis = currentMillis; // Atualiza tempo
  }
}