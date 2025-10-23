// ==================================================
// 🤖 AI Challenger – Reação automática a ID da HuskyLens
// --------------------------------------------------
// A HuskyLens está em modo Color Recognition (ou outro com IDs).
// Quando deteta um ID específico, o Arduino acende o LED correspondente:
// - ID 1: vira à esquerda (LED amarelo)
// - ID 2: vira à direita (LED azul)
// - ID 3: parar (LED vermelho)
// Outros IDs: ignora
// ==================================================

#include "HUSKYLENS.h"
#include "SoftwareSerial.h"

// Comunicação UART com a HuskyLens
SoftwareSerial mySerial(8, 9);  // RX, TX
HUSKYLENS huskylens;

// Pinos dos LEDs
const int ledVerde   = 4; // Sistema ativo
const int ledVermelho = 7; // Parar
const int ledAmarelo = 6; // Esquerda
const int ledAzul    = 5; // Direita

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  // Inicializar LEDs
  pinMode(ledVerde, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledAzul, OUTPUT);
  apagarTodosLEDs();

  // Mostrar estado ativo
  digitalWrite(ledVerde, HIGH);

  Serial.println("🎯 Sistema pronto. A aguardar deteções...");

  if (!huskylens.begin(mySerial)) {
    Serial.println("❌ Falha na comunicação com a HuskyLens.");
  } else {
    Serial.println("✅ HuskyLens ligada.");
  }
}

void loop() {
  if (huskylens.request()) {
    if (huskylens.available()) {
      HUSKYLENSResult result = huskylens.read();
      Serial.print("🆔 ID detetado: ");
      Serial.println(result.ID);

      // Apaga todos os LEDs antes de decidir
      apagarTodosLEDs();
      digitalWrite(ledVerde, HIGH); // Mantém LED de sistema ligado

      // Lógica de decisão por ID
      if (result.ID == 1) {
        Serial.println("↪️ Ação: Virar à ESQUERDA");
        digitalWrite(ledAmarelo, HIGH);
      }
      else if (result.ID == 2) {
        Serial.println("↩️ Ação: Virar à DIREITA");
        digitalWrite(ledAzul, HIGH);
      }
      else if (result.ID == 3) {
        Serial.println("⛔ Ação: PARAR");
        digitalWrite(ledVermelho, HIGH);
      }
      else if (result.ID == 4) {
        Serial.println(" Ação: FRENTE");
        digitalWrite(ledVerde, HIGH);
      }
      else {
        Serial.println("⚠️ ID não reconhecido.");
      }

    } else {
      Serial.println("⛔ Nada detetado.");
      apagarTodosLEDs();
      digitalWrite(ledVerde, HIGH); // Sistema ativo, mas sem deteção
    }
  } else {
    Serial.println("⚠️ Erro de comunicação com a HuskyLens.");
  }

  delay(500); // Aguarda meio segundo
}

void apagarTodosLEDs() {
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledVermelho, LOW);
  digitalWrite(ledAmarelo, LOW);
  digitalWrite(ledAzul, LOW);
}
