// ==============================================================================
// 🤖 AzoresBot AI CHALLENGER – Menu Serial para controlo de movimento com L298P
// ==============================================================================
// Permite escolher:
// - Velocidade (mesma para ambas as rodas)
// - Direção: frente, trás, virar esquerda ou direita
// - Duração do movimento (em ms)
// ==============================================================================

/* ===============================================================================
   ⚙️  AZORESBOT AI CHALLENGER – CONTROLO DE MOTORES COM DIFERENTES DRIVERS
   ===============================================================================
   Este código está configurado para o DRIVER **L298P**, onde cada motor usa:
      - 1 pino de DIREÇÃO  (M1 / M2)
      - 1 pino PWM (E1 / E2) para velocidade
   É o modelo usado nas shields “Arduino Motor Shield R3” e compatíveis.

   ➤ OUTRAS OPÇÕES DE DRIVERS DE MOTOR
   --------------------------------------------------------------------------------
   🟦 1. L298N “clássico” (com 2 pinos de direção por motor + PWM)
        - Cada motor tem IN1 / IN2 / ENA (direito)  e  IN3 / IN4 / ENB (esquerdo)
        - Frente:  IN1=HIGH, IN2=LOW   |   IN3=HIGH, IN4=LOW
        - Trás:    IN1=LOW,  IN2=HIGH  |   IN3=LOW,  IN4=HIGH
        - ENA/ENB ligam-se a pinos PWM do Arduino.
        - Para parar: analogWrite(ENA,0); analogWrite(ENB,0);

   🟪 2. TB6612FNG  (driver mais eficiente, recomendado para projetos maiores)
        - Cada motor tem AIN1/AIN2/PWMA  e  BIN1/BIN2/PWMB
        - Frente:  AIN1=HIGH, AIN2=LOW   |   BIN1=HIGH, BIN2=LOW
        - Trás:    AIN1=LOW,  AIN2=HIGH  |   BIN1=LOW,  BIN2=HIGH
        - O pino STBY deve ficar em HIGH (ou ligado a 5V).
        - Para “travagem rápida” (brake): AIN1=AIN2=HIGH + PWM=0.

   🟥 3. L293D (shields ou módulos antigos)
        - Muito semelhante ao L298N, mas suporta menos corrente.
        - Pode vir com jumpers que fixam ENA/ENB em HIGH (velocidade máxima).
        - Para controlo PWM, remove o jumper e liga ENA/ENB aos pinos PWM.

   🟧 4. Ponte H integrada em robôs educativos (p.ex. Maker UNO, RobotDyn, etc.)
        - Alguns modelos têm pinos nomeados como 1A/1B (motor A) e 2A/2B (motor B).
        - Verifica no datasheet se são “ativações diretas” (sem PWM)
          ou se tens pinos ENA/ENB separados.

   🟩 5. ESP32 e outras placas sem analogWrite()
        - O ESP32 usa “canal PWM” com funções:
             ledcAttachPin(pin, canal);
             ledcSetup(canal, freq, bits);
             ledcWrite(canal, duty);
        - Os pinos de direção continuam iguais.

   ➤ DICAS DE ADAPTAÇÃO
   -------------------------------------------------------------------------------
   • Se o carro andar PARA TRÁS quando mandas “frente” → troca HIGH/LOW do motor em questão.
   • Se o carro virar ao contrário → troca os cabos dos motores (direito ↔ esquerdo).
   • Alimenta SEMPRE os motores com fonte externa (6–12V) e liga o GND ao Arduino.
   • Para testes simples, “coast” (rodar livre) é suficiente. “Brake” (travagem rápida) é opcional.

   ➤ AJUSTE RÁPIDO NO CÓDIGO
   -------------------------------------------------------------------------------
   1) Mantém as funções: andarFrente(), andarTras(), virarEsquerda(), virarDireita().
   2) Dentro de cada uma, muda apenas o padrão de HIGH/LOW e os nomes dos pinos
      conforme o teu driver.
   3) Não precisas alterar o menu serial nem o resto do programa.
   ================================================================================================= */


// -------------------- Pinos e configuração --------------------
const int E1 = 10;  // PWM motor A (direita)
const int M1 = 12;  // Direção motor A
const int E2 = 11;  // PWM motor B (esquerda)
const int M2 = 13;  // Direção motor B

int velocidade = 150;      // Velocidade padrão (0–255)
int duracao = 1000;        // Duração padrão em ms

void setup() {
  Serial.begin(9600);
  pinMode(E1, OUTPUT); // Velocidade da roda DIREITA
  pinMode(M1, OUTPUT); // SENTIDO da roda DIREITA (1 FRENTE e 0 TRAS)
  pinMode(E2, OUTPUT); // Velocidade da roda ESQ
  pinMode(M2, OUTPUT); // SENTIDO da roda ESQ (1 FRENTE e 0 TRAS)
  parar();

  Serial.println("🚗 AzoresBot 2025 AI CHALLENGER – Menu de Movimento");
}

void loop() {
  mostrarMenu();

  while (!Serial.available()) delay(100);
  int opcao = Serial.readStringUntil('\n').toInt();

  switch (opcao) {
    case 1:
      Serial.print("⚙️ Nova velocidade (0–255): ");
      velocidade = lerValor();
      break;

    case 2:
      Serial.print("⏱️ Nova duração (ms): ");
      duracao = lerValor();
      break;

    case 3:
      Serial.println("⬆️ Movimento para a FRENTE");
      andarFrente(velocidade, duracao);
      break;

    case 4:
      Serial.println("⬇️ Movimento para TRÁS");
      andarTras(velocidade, duracao);
      break;

    case 5:
      Serial.println("↪️ Virar para a ESQUERDA");
      virarEsquerda(velocidade, duracao);
      break;

    case 6:
      Serial.println("↩️ Virar para a DIREITA");
      virarDireita(velocidade, duracao);
      break;

    case 0:
      mostrarParametros();
      break;

    default:
      Serial.println("❌ Opção inválida.");
      break;
  }

  limparBuffer();
}

// ------------------- Funções de Movimento -------------------

void andarTras(int vel, int tempo) {
  digitalWrite(M1, 1);  // F DIR
  digitalWrite(M2, 1);  // F ESQ
  analogWrite(E1, vel); // Velocidade DIR
  analogWrite(E2, vel);   // Velocidade ESQ
  delay(tempo);
  parar();
}

void andarFrente(int vel, int tempo) {
  digitalWrite(M1, 0);  // T DIR
  digitalWrite(M2, 0);  // T ESQ
  analogWrite(E1, vel);   // Velocidade DIR
  analogWrite(E2, vel); // Velocidade ESQ
  delay(tempo);
  parar();
}

void virarDireita(int vel, int tempo) {
  digitalWrite(M1, 1);  // F DIR
  digitalWrite(M2, 0);  // T ESQ
  analogWrite(E1, vel); // Velocidade DIR
  analogWrite(E2, vel); // Velocidade ESQ
  delay(tempo);
  parar();
}

void virarEsquerda(int vel, int tempo) {
  digitalWrite(M1, 0);  // T DIR
  digitalWrite(M2, 1);  // F ESQ
  analogWrite(E1, vel); // Velocidade DIR
  analogWrite(E2, vel); // Velocidade ESQ
  delay(tempo);
  parar();
}

void parar() {
  analogWrite(E1, 0);
  analogWrite(E2, 0);
  digitalWrite(M1, 0);
  digitalWrite(M2, 0);
}

// ------------------- Funções auxiliares -------------------

int lerValor() {
  while (!Serial.available()) delay(100);
  int val = Serial.readStringUntil('\n').toInt();
  Serial.println(val);
  return val;
}

void mostrarMenu() {
  Serial.println("\n========= MENU =========");
  Serial.println("1. Definir velocidade");
  Serial.println("2. Definir duração (ms)");
  Serial.println("3. Andar para FRENTE");
  Serial.println("4. Andar para TRÁS");
  Serial.println("5. Virar ESQUERDA");
  Serial.println("6. Virar DIREITA");
  Serial.println("0. Mostrar parâmetros atuais");
  Serial.print(">> ");
}

void mostrarParametros() {
  Serial.println("🔧 Parâmetros atuais:");
  Serial.print("Velocidade: "); Serial.println(velocidade);
  Serial.print("Duração: "); Serial.print(duracao); Serial.println(" ms");
}

void limparBuffer() {
  while (Serial.available()) Serial.read();
}
