 // ============================================================================
// 🤖 AzorCAR – Leitura de TAGS com HuskyLens (modo Tag Recognition)
// ----------------------------------------------------------------------------
// Usa SoftwareSerial (pinos 8 e 9) para comunicar com a HuskyLens
// Mostra a ID da TAG detetada no Monitor Serial
// Prepara estrutura para associar cada TAG a uma ação
// ============================================================================

#include "HUSKYLENS.h"
#include "SoftwareSerial.h"

SoftwareSerial huskySerial(8, 9); // RX = D8, TX = D9
HUSKYLENS huskylens;

void setup() {
  Serial.begin(9600);
  huskySerial.begin(9600);

  Serial.println("🔍 A iniciar HuskyLens (modo TAG Recognition)...");

  if (huskylens.begin(huskySerial)) {
    Serial.println("✅ Comunicação com HuskyLens OK.");
  } else {
    Serial.println("❌ ERRO: HuskyLens não encontrada.");
    while (1);
  }
}

void loop() {
  if (huskylens.request()) {
    if (huskylens.available()) {
      HUSKYLENSResult r = huskylens.read();

      // Mostra a TAG ID detetada
      Serial.print("🏷️ TAG Detetada → ID: ");
      Serial.println(r.ID);

      // Ações com base na TAG
      switch (r.ID) {
        case 1:
          Serial.println("➡️ Ação: ANDAR PARA A FRENTE");
          break;
        case 2:
          Serial.println("🛑 Ação: PARAR");
          break;
        case 3:
          Serial.println("↪️ Ação: VIRAR ESQUERDA");
          break;
        case 4:
          Serial.println("↩️ Ação: VIRAR DIREITA");
          break;
        case 5:
          Serial.println("⬇️ Ação: ANDAR PARA TRÁS");
          break;
        case 6:
          Serial.println("🎯 Ação: MODO SERVO / CÂMARA");
          break;
        default:
          Serial.println("❓ TAG sem ação definida.");
          break;
      }
    } else {
      Serial.println("⚠️ Sem TAG visível.");
    }
  } else {
    Serial.println("❌ ERRO: Comunicação falhou.");
  }

  delay(300); // Pausa para evitar spam
}

