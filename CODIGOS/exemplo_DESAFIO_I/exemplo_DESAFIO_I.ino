// =================================================================================
// 🤖 AI Challenger – Desafio de Navegação por Tags 
// 🏆 Desafio I Azoresbot 2025 – Reconhecimento de 4 Tags com HuskyLens
// 📷 HuskyLens: Reconhecimento de Tags (ID1 a ID4) via UART
// 📺 OLED SH1106: Exibe estados, contagens e ações do carro
// 💡 LEDs Indicativos: Verde (TAG1), Amarelo (TAG2), Azul (TAG3), Vermelho (TAG4)
// 🚗 Controle de Motores: Driver L298P com movimentos discretos
// 🔄 Funcionalidade: Rotação inicial, busca de TAGs, aproximação, pausas e fim
// 🕒 Comportamento: Pausa inicial de 5s, pausas de 10s após TAG1/2/3, fim na TAG4
// 🔍 Busca Lenta: Gira suavemente (200ms) com pausas de 1000ms, avança a cada 3 rotações, timeout de 30s
// ================================================================================

#include <SoftwareSerial.h>
#include "HUSKYLENS.h"
#include <Wire.h>
#include <U8x8lib.h>

// === Comunicação com HuskyLens via UART ===
SoftwareSerial huskySerial(8, 9); // RX: 8, TX: 9
HUSKYLENS huskylens;

// === OLED SH1106 ===
U8X8_SH1106_128X64_NONAME_HW_I2C oled(U8X8_PIN_NONE);

// === Pinos do driver L298P ===
const int E1 = 10; // PWM direita
const int M1 = 12; // DIR direita
const int E2 = 11; // PWM esquerda
const int M2 = 13; // DIR esquerda

// === Pinos dos LEDs indicativos ===
const int ledVerde = 4;
const int ledAmarelo = 6;
const int ledAzul = 5;
const int ledVermelho = 7;

// === IDs das TAGs ===
const int ID1 = 1;
const int ID2 = 2;
const int ID3 = 3;
const int ID4 = 4;
int ID_ATUAL = ID1;

// === Calibração por ID ===
const int W_MIN = 75;
const int W_MAX = 120;
const int H_MIN = 75;
const int H_MAX = 116;
const int LIMIAR_X = 15;

// === Movimento ===
const int VEL_ROTACAO = 130; // Velocidade para rotação inicial
const int TEMPO_ROTACAO = 180; // Tempo para rotação inicial
const int PAUSA_ROTACAO = 500; // Pausa entre rotações iniciais
const int PASSOS_INICIAIS = 12; // Número de passos na rotação inicial
const int VEL_AVANCO = 140; // Velocidade de avanço
const int TEMPO_AVANCO = 150; // Tempo de avanço
const int VEL_RECUO = 130; // Velocidade de recuo
const int TEMPO_RECUO = 150; // Tempo de recuo
const int VEL_CORRIGIR = 100; // Velocidade para correção de direção
const int TEMPO_CORRIGIR = 100; // Tempo para correção
const int VEL_ROTACAO_LENTA = 120; // Velocidade para busca lenta
const int TEMPO_ROTACAO_LENTA = 200; // Tempo de rotação na busca
const int PAUSA_LEITURA = 1000; // Pausa para leitura do HuskyLens
const int ROTACOES_POR_CICLO = 3; // Rotações antes de avançar na busca

// === Estados ===
bool rotacaoInicialFeita = false;
bool ultimaDirecaoDireita = true;

// === Controle do piscar do LED vermelho ===
unsigned long previousMillis = 0;
const long blinkInterval = 500;
bool ledVermelhoState = false;

// === Adições para robustez ===
const int PIN_REINICIO = 2; // Pino para reiniciar (opcional)
const int MAX_TENTATIVAS_CONEXAO = 5; // Limite de tentativas de conexão
const int MAX_TENTATIVAS_BUSCA = 3; // Limite de tentativas de busca

// === Funções de movimento ===
void parar() {
  analogWrite(E1, 0); analogWrite(E2, 0);
  digitalWrite(M1, LOW); digitalWrite(M2, LOW);
  oled.clearLine(3);
  oled.setCursor(0, 3);
  oled.print("Parado");
  apagarLEDsExcetoVermelho();
  piscarLEDVermelho();
}

void frente(int vel, int tempo) {
  digitalWrite(M1, LOW); digitalWrite(M2, LOW); // Ajustado para frente
  analogWrite(E1, vel); analogWrite(E2, vel);
  oled.clearLine(3);
  oled.setCursor(0, 3);
  oled.print("Frente");
  apagarLEDsExcetoVermelho();
  delay(tempo);
  parar();
}

void tras(int vel, int tempo) {
  digitalWrite(M1, HIGH); digitalWrite(M2, HIGH); // Ajustado para trás
  analogWrite(E1, vel); analogWrite(E2, vel);
  oled.clearLine(3);
  oled.setCursor(0, 3);
  oled.print("Tras");
  apagarLEDsExcetoVermelho();
  delay(tempo);
  parar();
}

void girar_direita(int vel, int tempo) {
  digitalWrite(M1, HIGH); digitalWrite(M2, LOW); // Ajustado para direita
  analogWrite(E1, vel); analogWrite(E2, vel);
  oled.clearLine(3);
  oled.setCursor(0, 3);
  oled.print("Direita");
  apagarLEDsExcetoVermelho();
  delay(tempo);
  parar();
}

void girar_esquerda(int vel, int tempo) {
  digitalWrite(M1, LOW); digitalWrite(M2, HIGH); // Ajustado para esquerda
  analogWrite(E1, vel); analogWrite(E2, vel);
  oled.clearLine(3);
  oled.setCursor(0, 3);
  oled.print("Esquerda");
  apagarLEDsExcetoVermelho();
  delay(tempo);
  parar();
}

void apagarLEDsExcetoVermelho() {
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledAmarelo, LOW);
  digitalWrite(ledAzul, LOW);
}

void piscarLEDVermelho() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= blinkInterval) {
    ledVermelhoState = !ledVermelhoState;
    digitalWrite(ledVermelho, ledVermelhoState ? HIGH : LOW);
    previousMillis = currentMillis;
  }
}

void buscaTAGLenta(unsigned long tempoLimite, int idDesejado) {
  unsigned long tempoInicio = millis();
  bool direcao = ultimaDirecaoDireita;
  int rotacoesContadas = 0;

  while (millis() - tempoInicio < tempoLimite) {
    piscarLEDVermelho();
    if (direcao) {
      girar_direita(VEL_ROTACAO_LENTA, TEMPO_ROTACAO_LENTA);
    } else {
      girar_esquerda(VEL_ROTACAO_LENTA, TEMPO_ROTACAO_LENTA);
    }
    rotacoesContadas++;
    ultimaDirecaoDireita = !ultimaDirecaoDireita;
    delay(PAUSA_LEITURA);

    if (huskylens.request() && huskylens.available()) {
      HUSKYLENSResult r = huskylens.read();
      if (r.ID == idDesejado) {
        parar();
        return;
      }
    }

    if (rotacoesContadas >= ROTACOES_POR_CICLO) {
      frente(VEL_AVANCO, TEMPO_AVANCO);
      delay(500); // Pausa extra após avanço
      rotacoesContadas = 0;
      delay(PAUSA_LEITURA);
      if (huskylens.request() && huskylens.available()) {
        HUSKYLENSResult r = huskylens.read();
        if (r.ID == idDesejado) {
          parar();
          return;
        }
      }
    }
  }
  oled.clear();
  oled.drawString(0, 0, "TAG ");
  oled.print(idDesejado);
  oled.drawString(0, 1, "perdida 30s");
  while (true) {
    piscarLEDVermelho();
    if (digitalRead(PIN_REINICIO) == LOW) asm volatile ("jmp 0");
    delay(100);
  }
}

void setup() {
  // Inicializa comunicação com HuskyLens com verificação robusta
  huskySerial.begin(9600);
  int tentativasConexao = 0;
  while (!huskylens.begin(huskySerial) && tentativasConexao < MAX_TENTATIVAS_CONEXAO) {
    oled.begin();
    oled.setFont(u8x8_font_5x7_f);
    oled.drawString(0, 0, "Erro HuskyLens");
    delay(1000);
    tentativasConexao++;
  }
  if (tentativasConexao >= MAX_TENTATIVAS_CONEXAO) {
    oled.clear();
    oled.drawString(0, 0, "Erro Fatal");
    while (true) {
      piscarLEDVermelho();
      if (digitalRead(PIN_REINICIO) == LOW) asm volatile ("jmp 0");
      delay(100);
    }
  }
  huskylens.writeAlgorithm(ALGORITHM_TAG_RECOGNITION);

  // Inicializa OLED
  oled.begin();
  oled.setFont(u8x8_font_5x7_f);
  oled.drawString(0, 0, "AzorCAR Iniciado");

  // Configura pinos
  pinMode(E1, OUTPUT); pinMode(M1, OUTPUT);
  pinMode(E2, OUTPUT); pinMode(M2, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledAzul, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(PIN_REINICIO, INPUT_PULLUP);

  digitalWrite(ledVerde, LOW);
  digitalWrite(ledAmarelo, LOW);
  digitalWrite(ledAzul, LOW);
  digitalWrite(ledVermelho, LOW);

  parar();
  previousMillis = millis();

  // Pausa inicial de 5s com contagem regressiva
  oled.clear();
  oled.drawString(0, 0, "Iniciando em:");
  for (int i = 5; i >= 0; i--) {
    oled.clearLine(1);
    oled.setCursor(0, 1);
    oled.print(i); oled.print(" s");
    delay(1000);
  }
  oled.clear();
  oled.drawString(0, 0, "Busca TAG ");
  oled.print(ID_ATUAL);

  // Rotação inicial
  oled.clearLine(3);
  oled.setCursor(0, 3);
  oled.print("Rotacao Inicial");
  for (int i = 0; i < PASSOS_INICIAIS; i++) {
    girar_direita(VEL_ROTACAO, TEMPO_ROTACAO);
    delay(PAUSA_ROTACAO);
  }
  oled.clear();
  oled.drawString(0, 0, "Rotacao Completa");
  oled.drawString(0, 1, "Busca TAG ");
  oled.print(ID_ATUAL);
  rotacaoInicialFeita = true;

  // Verificação estática após rotação
  oled.clearLine(2);
  oled.setCursor(0, 2);
  oled.print("Verifica TAG");
  delay(PAUSA_LEITURA);
  huskylens.request();
  if (huskylens.available()) {
    HUSKYLENSResult r = huskylens.read();
    if (r.ID == ID_ATUAL) {
      oled.clearLine(2);
      oled.setCursor(0, 2);
      oled.print("TAG encontrada!");
      delay(500);
      return;
    }
  }
}

void loop() {
  const unsigned long timeout = 30000;
  int tentativas = 0;

  // Busca a tag atual
  buscaTAGLenta(timeout, ID_ATUAL);

  // Aproximação à TAG (usando lógica do HuskyCar)
  unsigned long startTime = millis();
  while (millis() - startTime < timeout) {
    if (millis() - startTime >= timeout) {
      oled.clear();
      oled.drawString(0, 0, "TAG ");
      oled.print(ID_ATUAL);
      oled.drawString(0, 1, "perdida 30s");
      while (true) {
        piscarLEDVermelho();
        if (digitalRead(PIN_REINICIO) == LOW) asm volatile ("jmp 0");
        delay(100);
      }
    }
    if (!huskylens.request() || !huskylens.available()) {
      if (tentativas < MAX_TENTATIVAS_BUSCA) {
        tentativas++;
        delay(500);
        continue;
      }
      buscaTAGLenta(timeout, ID_ATUAL);
      startTime = millis();
      tentativas = 0;
      continue;
    }
    HUSKYLENSResult r = huskylens.read();
    if (r.ID != ID_ATUAL) {
      buscaTAGLenta(timeout, ID_ATUAL);
      startTime = millis();
      continue;
    }
    int x = r.xCenter - 160;
    int w = r.width;
    int h = r.height;
    oled.clearLine(2);
    oled.setCursor(0, 2);
    oled.print("ID"); oled.print(ID_ATUAL); oled.print(" X:");
    oled.print(x > 0 ? "+" : "");
    oled.print(constrain(x, -99, 99));

    if (x < -LIMIAR_X) {
      oled.clearLine(3);
      oled.setCursor(0, 3);
      oled.print("Corrigir: ESQ");
      girar_esquerda(VEL_CORRIGIR, TEMPO_CORRIGIR);
    } else if (x > LIMIAR_X) {
      oled.clearLine(3);
      oled.setCursor(0, 3);
      oled.print("Corrigir: DIR");
      girar_direita(VEL_CORRIGIR, TEMPO_CORRIGIR);
    } else {
      if (w < W_MIN || h < H_MIN) {
        oled.clearLine(3);
        oled.setCursor(0, 3);
        oled.print("Avancar");
        frente(VEL_AVANCO, TEMPO_AVANCO);
      } else if (w > W_MAX || h > H_MAX) {
        oled.clearLine(3);
        oled.setCursor(0, 3);
        oled.print("Recuar");
        tras(VEL_RECUO, TEMPO_RECUO);
      } else {
        oled.clearLine(3);
        oled.setCursor(0, 3);
        oled.print("Parado (ideal)");
        parar();
        apagarLEDsExcetoVermelho();

        if (ID_ATUAL == ID1) {
          digitalWrite(ledVerde, HIGH);
          ID_ATUAL = ID2;
        } else if (ID_ATUAL == ID2) {
          digitalWrite(ledAmarelo, HIGH);
          ID_ATUAL = ID3;
        } else if (ID_ATUAL == ID3) {
          digitalWrite(ledAzul, HIGH);
          ID_ATUAL = ID4;
        } else if (ID_ATUAL == ID4) {
          digitalWrite(ledVermelho, HIGH);
          oled.clear();
          oled.drawString(0, 0, "TAG 4 OK");
          oled.drawString(0, 1, "Fim da Prova");
          while (true) {
            piscarLEDVermelho();
            if (digitalRead(PIN_REINICIO) == LOW) asm volatile ("jmp 0");
            delay(100);
          }
        }

        oled.clear();
        oled.drawString(0, 0, "Pausa:");
        for (int i = 10; i >= 0; i--) {
          oled.clearLine(1);
          oled.setCursor(0, 1);
          oled.print(i); oled.print(" s");
          piscarLEDVermelho();
          delay(1000);
        }
        digitalWrite(ledVerde, LOW);
        digitalWrite(ledAmarelo, LOW);
        digitalWrite(ledAzul, LOW);

        oled.clear();
        oled.drawString(0, 0, "Busca TAG ");
        oled.print(ID_ATUAL);
        break;
      }
    }
    delay(200); // Manter delay curto para evitar travamento
  }
}