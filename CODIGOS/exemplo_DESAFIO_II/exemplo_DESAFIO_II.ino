// ===============================================================
// 🤖 AI Challenger – Navegação em Labirinto com Tags
// 🏆 Desafio Azoresbot – Busca de 4 Tags em Labirinto
// 📷 HuskyLens: Reconhecimento de Tags (ID1 a ID4) em qualquer ordem
// 📡 Sensores HC-SR04: 3 sensores (frontal, direita, esquerda) para navegação
// 📺 OLED SH1106: Exibe contagem inicial, estados e ações
// 💡 LEDs Indicativos: Verde (TAG1), Amarelo (TAG2), Azul (TAG3), Vermelho (TAG4)
// 🚗 Controle de Motores: Driver L298P com movimentos discretos
// 🔄 Funcionalidade: Navega em labirinto, encontra TAGs, acende LEDs, termina após 4 TAGs
// 🕒 Comportamento: Pausa inicial de 5s, navegação com sensores, busca de TAGs
// ===============================================================

#include <SoftwareSerial.h>
#include <HUSKYLENS.h>
#include <Wire.h>
#include <U8x8lib.h>
#include <NewPing.h>

// === Configurações ===
#define TRIGGER_FRONTAL 2
#define ECHO_FRONTAL 3
#define TRIGGER_DIREITA 4
#define ECHO_DIREITA 5
#define TRIGGER_ESQUERDA 6
#define ECHO_ESQUERDA 7
#define MAX_DISTANCIA 200 // cm

// === Componentes ===
SoftwareSerial huskySerial(8, 9);
HUSKYLENS huskylens;
U8X8_SH1106_128X64_NONAME_HW_I2C oled(U8X8_PIN_NONE);
NewPing sonarFrontal(TRIGGER_FRONTAL, ECHO_FRONTAL, MAX_DISTANCIA);
NewPing sonarDireita(TRIGGER_DIREITA, ECHO_DIREITA, MAX_DISTANCIA);
NewPing sonarEsquerda(TRIGGER_ESQUERDA, ECHO_ESQUERDA, MAX_DISTANCIA);

// === Pinos do L298P ===
const int E1 = 10; // PWM direita
const int M1 = 12; // DIR direita
const int E2 = 11; // PWM esquerda
const int M2 = 13; // DIR esquerda

// === LEDs ===
const int ledVerde = A0;    // TAG1
const int ledAmarelo = A1;  // TAG2
const int ledAzul = A2;     // TAG3
const int ledVermelho = A3; // TAG4

// === IDs das TAGs ===
const int ID1 = 1;
const int ID2 = 2;
const int ID3 = 3;
const int ID4 = 4;

// === Parâmetros ===
const int DIST_MIN_FRONTAL = 20; // cm
const int DIST_MIN_LATERAL = 30; // cm
const int VEL_MOV = 140;
const int TEMPO_MOV = 150;
const int VEL_GIRO = 100;
const int TEMPO_GIRO = 100;
const int W_MIN = 75;
const int W_MAX = 120;
const int H_MIN = 75;
const int H_MAX = 116;
const int LIMIAR_X = 15;

// === Estados ===
bool tagEncontrada[4] = {false, false, false, false};
int tagsEncontradas = 0;

// === Funções de Movimento ===
void parar() { /* Desliga motores, exibe "Parado" no OLED */ }
void frente() { /* Move à frente, exibe "Frente" */ }
void girarDireita() { /* Gira à direita, exibe "Direita" */ }
void girarEsquerda() { /* Gira à esquerda, exibe "Esquerda" */ }

// === Setup ===
void setup() {
  // Inicializa HuskyLens
  huskySerial.begin(9600);
  while (!huskylens.begin(huskySerial)) {
    oled.begin();
    oled.setFont(u8x8_font_5x7_f);
    oled.drawString(0, 0, "Erro HuskyLens");
    delay(1000);
  }
  huskylens.writeAlgorithm(ALGORITHM_TAG_RECOGNITION);

  // Inicializa OLED
  oled.begin();
  oled.setFont(u8x8_font_5x7_f);
  oled.drawString(0, 0, "AzorCAR Iniciado");

  // Configura pinos
  pinMode(E1, OUTPUT); pinMode(M1, OUTPUT);
  pinMode(E2, OUTPUT); pinMode(M2, OUTPUT);
  pinMode(ledVerde, OUTPUT); pinMode(ledAmarelo, OUTPUT);
  pinMode(ledAzul, OUTPUT); pinMode(ledVermelho, OUTPUT);
  digitalWrite(ledVerde, LOW); digitalWrite(ledAmarelo, LOW);
  digitalWrite(ledAzul, LOW); digitalWrite(ledVermelho, LOW);

  // Contagem inicial
  oled.clear();
  oled.drawString(0, 0, "Iniciando em:");
  for (int i = 5; i >= 0; i--) {
    oled.clearLine(1);
    oled.setCursor(0, 1);
    oled.print(i);
    oled.print(" s");
    delay(1000);
  }
  oled.clear();
  oled.drawString(0, 0, "Busca TAGs");
}

// === Loop Principal ===
void loop() {
  if (tagsEncontradas == 4) {
    oled.clear();
    oled.drawString(0, 0, "Fim da Prova");
    parar();
    while (true) delay(100);
  }

  // Lê sensores de distância
  unsigned long distFrontal = sonarFrontal.ping_cm();
  unsigned long distDireita = sonarDireita.ping_cm();
  unsigned long distEsquerda = sonarEsquerda.ping_cm();

  // Verifica TAGs com HuskyLens
  if (huskylens.request() && huskylens.available()) {
    HUSKYLENSResult r = huskylens.read();
    if (r.ID >= ID1 && r.ID <= ID4 && !tagEncontrada[r.ID - 1]) {
      oled.clearLine(2);
      oled.setCursor(0, 2);
      oled.print("TAG ");
      oled.print(r.ID);
      oled.print(" OK");

      // Centraliza e aproxima
      int x = r.xCenter - 160;
      int w = r.width;
      int h = r.height;
      if (x < -LIMIAR_X) girarEsquerda();
      else if (x > LIMIAR_X) girarDireita();
      else if (w < W_MIN || h < H_MIN) frente();
      else if (w > W_MAX || h > H_MAX) /* recuar */;
      else {
        // TAG na zona ideal
        tagEncontrada[r.ID - 1] = true;
        tagsEncontradas++;
        if (r.ID == ID1) digitalWrite(ledVerde, HIGH);
        else if (r.ID == ID2) digitalWrite(ledAmarelo, HIGH);
        else if (r.ID == ID3) digitalWrite(ledAzul, HIGH);
        else if (r.ID == ID4) digitalWrite(ledVermelho, HIGH);
        oled.clear();
        oled.drawString(0, 0, "TAG ");
        oled.print(r.ID);
        oled.drawString(0, 1, "Encontrada");
        delay(1000); // Pausa breve para feedback
        oled.clear();
        oled.drawString(0, 0, "Busca TAGs");
      }
    }
  } else {
    oled.clearLine(2);
    oled.setCursor(0, 2);
    oled.print("Perdido");
  }

  // Navegação no labirinto (mão na parede à direita)
  if (distFrontal < DIST_MIN_FRONTAL && distFrontal > 0) {
    if (distDireita > DIST_MIN_LATERAL) girarDireita();
    else if (distEsquerda > DIST_MIN_LATERAL) girarEsquerda();
    else /* recuar ou girar */;
  } else {
    frente();
  }
}