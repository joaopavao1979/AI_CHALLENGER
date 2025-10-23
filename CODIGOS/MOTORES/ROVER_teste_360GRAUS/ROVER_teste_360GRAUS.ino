// ============================================================================
//              ROBÔ HUSKYCAR – MENU DE ROTAÇÃO AVANÇADA
// ============================================================================
// Autor: João Pavão
// Descrição: Controlo independente das rodas com menu interativo via Serial.
// Permite definir velocidade e tempo de cada motor, número de passos e sentido.
// ============================================================================

// ----------------------------
// 1. Mapeamento dos pinos
// ----------------------------
const int E1 = 10;  // PWM motor A (direita)
const int M1 = 12;  // Direção motor A
const int E2 = 11;  // PWM motor B (esquerda)
const int M2 = 13;  // Direção motor B

// ----------------------------
// 2. Parâmetros configuráveis
// ----------------------------
int velA = 200;                // Velocidade motor A (0–255)
int velB = 200;                // Velocidade motor B (0–255)
int tempoA = 300;              // Tempo motor A (ms)
int tempoB = 300;              // Tempo motor B (ms)
int passos = 12;               // Nº de passos (ex: 12 → 360º)
bool sentidoHorario = true;   // Direção da rotação

// ============================================================================
// 3. Setup
// ============================================================================
void setup() {
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);
  pinMode(E1, OUTPUT);
  pinMode(E2, OUTPUT);

  Serial.begin(9600);
  delay(1000);
  Serial.println("\n=== HUSKYCAR – MENU DE ROTAÇÃO AVANÇADA ===");
}

// ============================================================================
// 4. Loop principal
// ============================================================================
void loop() {
  mostrarParametros();
  mostrarMenu();

  while (!Serial.available()) delay(100);
  int opcao = Serial.readStringUntil('\n').toInt();

  switch (opcao) {
    case 1: Serial.print("Velocidade Motor A: "); velA = lerValor(); break;
    case 2: Serial.print("Velocidade Motor B: "); velB = lerValor(); break;
    case 3: Serial.print("Tempo Motor A (ms): "); tempoA = lerValor(); break;
    case 4: Serial.print("Tempo Motor B (ms): "); tempoB = lerValor(); break;
    case 5: Serial.print("Nº de passos: "); passos = lerValor(); break;
    case 6: sentidoHorario = !sentidoHorario; Serial.println("Sentido invertido!"); break;
    case 7: executarRotacao(); break;
    case 8: mostrarParametros(); break;
    case 0: break;
    default: Serial.println("Opção inválida!"); break;
  }
  while (Serial.available()) Serial.read();
}

// ============================================================================
// 5. Funções auxiliares
// ============================================================================
void mostrarMenu() {
  Serial.println("\n--- MENU ---");
  Serial.println("[1] Alterar velocidade Motor A (direita)");
  Serial.println("[2] Alterar velocidade Motor B (esquerda)");
  Serial.println("[3] Alterar tempo Motor A (ms)");
  Serial.println("[4] Alterar tempo Motor B (ms)");
  Serial.println("[5] Alterar nº de passos");
  Serial.println("[6] Alternar sentido de rotação");
  Serial.println("[7] Executar rotação");
  Serial.println("[8] Mostrar parâmetros");
  Serial.println("[0] Repetir menu");
  Serial.print(">> ");
}

void mostrarParametros() {
  Serial.println("\n--- PARÂMETROS ATUAIS ---");
  Serial.print("Velocidade Motor A: "); Serial.println(velA);
  Serial.print("Velocidade Motor B: "); Serial.println(velB);
  Serial.print("Tempo Motor A: "); Serial.print(tempoA); Serial.println(" ms");
  Serial.print("Tempo Motor B: "); Serial.print(tempoB); Serial.println(" ms");
  Serial.print("Passos: "); Serial.println(passos);
  Serial.print("Sentido: "); Serial.println(sentidoHorario ? "HORÁRIO" : "ANTI-HORÁRIO");
}

int lerValor() {
  while (!Serial.available()) delay(100);
  int valor = Serial.readStringUntil('\n').toInt();
  Serial.println(valor);
  return valor;
}

// ============================================================================
// 6. Execução da rotação com parâmetros atuais
// ============================================================================
void executarRotacao() {
  Serial.println("\n>>> Iniciando rotação...");

  for (int i = 0; i < passos; i++) {
    Serial.print("🔁 Passo "); Serial.println(i + 1);

    if (sentidoHorario) {
      digitalWrite(M1, HIGH); // frente
      digitalWrite(M2, LOW);  // trás
    } else {
      digitalWrite(M1, LOW);  // trás
      digitalWrite(M2, HIGH); // frente
    }

    analogWrite(E1, velA);
    analogWrite(E2, velB);
    delay(max(tempoA, tempoB)); // duração maior como base

    parar();
    delay(400); // pequena pausa entre passos
  }

  Serial.println("✅ Rotação concluída.");
}

void parar() {
  analogWrite(E1, 0);
  analogWrite(E2, 0);
  digitalWrite(M1, LOW);
  digitalWrite(M2, LOW);
}
