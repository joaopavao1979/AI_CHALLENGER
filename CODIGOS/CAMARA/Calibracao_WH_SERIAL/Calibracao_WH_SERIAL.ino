// ==============================================================================
// 🎯 AI Challenge – Calibração de Proximidade (HuskyLens – Modo Cor)
// ==============================================================================
// Objetivo:
// Permitir ao utilizador calibrar manualmente os limites de distância entre
// o robô e o marcador visual (cor com ID treinado na HuskyLens).
//
// Funcionamento:
// - Mostra no Serial Monitor os valores de:
//      • ID (identificador da cor detetada)
//      • W (largura do retângulo de deteção)
//      • H (altura do retângulo de deteção)
//      • X e Y (posição relativa ao centro da imagem)
//
// Instruções:
// 1. Posicionar o robô parado no chão.
// 2. Colocar um marcador colorido à frente do robô:
//      → a 15 cm (para determinar os valores mínimos de W e H)
//      → a 25 cm (para determinar os valores máximos de W e H)
// 3. Apontar a HuskyLens para o marcador e ler os valores no Serial Monitor.
// 4. Usar esses valores como limites de paragem no código principal.
//
// Observação:
// - Só funciona com uma cor de cada vez (modo Color Recognition).
// - Ideal para configurar `W_MIN`, `W_MAX`, `H_MIN` e `H_MAX`.
// ==============================================================================

#include <SoftwareSerial.h>
#include <HUSKYLENS.h>

SoftwareSerial huskySerial(8, 9); // UART: RX = 8, TX = 9
HUSKYLENS huskylens;

void setup() {
  Serial.begin(9600);
  huskySerial.begin(9600);

  Serial.println("=====================================================");
  Serial.println("📏 Calibracao de Proximidade – HuskyLens (modo cor)");
  Serial.println("-----------------------------------------------------");
  Serial.println("1. Coloca o carro fixo no chao.");
  Serial.println("2. Coloca um marcador (cor ID1) a:");
  Serial.println("   👉 15 cm → regista os valores de W e H = MIN");
  Serial.println("   👉 25 cm → regista os valores de W e H = MAX");
  Serial.println("3. Os dados aparecem abaixo.");
  Serial.println("⚠️ So é considerada a primeira deteccao (uma cor de cada vez).");
  Serial.println("=====================================================");
  Serial.println();

  if (!huskylens.begin(huskySerial)) {
    Serial.println("Erro: HuskyLens nao encontrada!");
    while (1);
  }
}

void loop() {
  if (huskylens.request() && huskylens.available()) {
    HUSKYLENSResult r = huskylens.read();
    Serial.print("ID: ");
    Serial.print(r.ID);
    Serial.print(" | W: ");
    Serial.print(r.width);
    Serial.print(" | H: ");
    Serial.print(r.height);
    Serial.print(" | X: ");
    Serial.print(r.xCenter - 160);
    Serial.print(" | Y: ");
    Serial.println(120 - r.yCenter);
  } else {
    Serial.println("Sem deteccao...");
  }

  delay(500); // tempo entre leituras
}
