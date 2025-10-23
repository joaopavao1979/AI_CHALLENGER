// ===============================================================
// 🤖 HuskyLens — Reconhecimento de "Sinais" (Object Recognition)
//     STOP (ID1), DIREITA (ID2), ESQUERDA (ID3), FRENTE (ID4 opcional)
// Comunicação: SoftwareSerial (D8=RX, D9=TX)
// Saída: Mensagens no Monitor Serial com a ação sugerida
// ---------------------------------------------------------------
// Como usar:
// 1) Na HuskyLens, seleciona o modo "Object Recognition".
// 2) Treina 3 (ou 4) classes: STOP, DIREITA, ESQUERDA (e FRENTE).
// 3) Garante que os IDs ficam mapeados como: 1=STOP, 2=DIREITA, 3=ESQUERDA, 4=FRENTE.
// 4) Corre este código e observa as ações no Serial.
// ===============================================================

#include <SoftwareSerial.h>
#include "HUSKYLENS.h"

SoftwareSerial huskySerial(8, 9); // RX=D8 (para Arduino), TX=D9
HUSKYLENS huskylens;

// ---- (Opcional) anti-ruído: requer X leituras iguais antes de "confirmar" ----
const int VOTOS_PARA_CONFIRMAR = 3;
int idAnterior = -1;
int contagemVotos = 0;

// ---- (Opcional) temporização para não repetir a mesma ação em loop ----
unsigned long ultimoAcionamentoMs = 0;
const unsigned long intervaloAcaoMs = 800; // 0.8 s entre ações idênticas

// ---- Mapeamento dos IDs para "sinais" ----
enum Sinal {
  SINAL_DESCONHECIDO = 0,
  SINAL_STOP         = 1, // ID1
  SINAL_DIREITA      = 2, // ID2
  SINAL_ESQUERDA     = 3, // ID3
  SINAL_FRENTE       = 4  // ID4 (opcional)
};

// ---------- AÇÕES (por agora só Serial; depois liga motores/LEDs aqui) ----------
void acaoSTOP() {
  Serial.println("🟥 AÇÃO: STOP (parar motores)");
  // TODO: parar motores, travar, LED vermelho, buzzer, etc.
}

void acaoDIREITA() {
  Serial.println("➡️  AÇÃO: VIRAR DIREITA");
  // TODO: sequencia de viragem à direita (ex.: motorE frente, motorD trás por X ms)
}

void acaoESQUERDA() {
  Serial.println("⬅️  AÇÃO: VIRAR ESQUERDA");
  // TODO: sequencia de viragem à esquerda
}

void acaoFRENTE() {
  Serial.println("⬆️  AÇÃO: SEGUIR EM FRENTE");
  // TODO: ambos os motores para a frente a velocidade segura
}

// Decide e executa a ação com base no ID reconhecido
void executarAcaoPorID(int id) {
  unsigned long agora = millis();
  if (agora - ultimoAcionamentoMs < intervaloAcaoMs) return; // evita spam

  switch (id) {
    case SINAL_STOP:     acaoSTOP();     break;
    case SINAL_DIREITA:  acaoDIREITA();  break;
    case SINAL_ESQUERDA: acaoESQUERDA(); break;
    case SINAL_FRENTE:   acaoFRENTE();   break;
    default:
      // ID desconhecido/nao mapeado
      break;
  }
  ultimoAcionamentoMs = agora;
}

void setup() {
  Serial.begin(9600);
  huskySerial.begin(9600);

  Serial.println("🔌 A iniciar HuskyLens (Object Recognition)...");
  if (!huskylens.begin(huskySerial)) {
    Serial.println("❌ ERRO: HuskyLens não encontrada. Verifica cabos/pinos 8-9.");
    while (1);
  }

  // (Opcional) Forçar algoritmo pelo protocolo — útil se a câmara ficou noutro modo
  huskylens.writeAlgorithm(ALGORITHM_OBJECT_RECOGNITION);
  Serial.println("✅ HuskyLens OK. Modo: Object Recognition");
  Serial.println("👉 Mapeia assim: ID1=STOP, ID2=DIREITA, ID3=ESQUERDA, ID4=FRENTE(opc.)");
}

void loop() {
  // Pede à HuskyLens os resultados do frame atual
  if (!huskylens.request()) {
    // sem dados; segue
    return;
  }

  // Se houver deteções, podemos ler "blocos" (objetos) reconhecidos
  if (huskylens.available()) {
    // Estratégia simples: usar o PRIMEIRO bloco (ou o maior, se quiseres)
    // Para o maior, teríamos de iterar e escolher por área (w*h).
    HUSKYLENSResult r = huskylens.read();

    int id = r.ID; // ID da classe treinada
    if (id <= 0) return; // sem ID válido

    // ---- Anti-ruído: confirma a mesma leitura X vezes seguidas ----
    if (id == idAnterior) {
      contagemVotos++;
    } else {
      idAnterior = id;
      contagemVotos = 1;
    }

    if (contagemVotos >= VOTOS_PARA_CONFIRMAR) {
      // Confirmado: executa ação associada ao ID
      switch (id) {
        case SINAL_STOP:
          Serial.println("🔎 Detetado: STOP (ID1)");
          break;
        case SINAL_DIREITA:
          Serial.println("🔎 Detetado: DIREITA (ID2)");
          break;
        case SINAL_ESQUERDA:
          Serial.println("🔎 Detetado: ESQUERDA (ID3)");
          break;
        case SINAL_FRENTE:
          Serial.println("🔎 Detetado: FRENTE (ID4)");
          break;
        default:
          Serial.print("🔎 Detetado: ID ");
          Serial.println(id);
          break;
      }
      executarAcaoPorID(id);
      contagemVotos = 0; // reinicia votos para próximo comando
    }
  }
}
