// ===============================================
// 🚗 CARRO ROBÔ LABIRINTO - REGRA DA DIREITA COM CORREÇÃO DE CENTRO
// Projeto: AI CHALLENGER - AzoresBot
// Baseado em: 3_sensores_serial_monitor.ino, AI_SEGUE_LINHA_v1.ino, AI_CHALLENGER_MOVIMENTO.ino
// Funcionalidade: Resolve labirinto seguindo a "regra da mão direita" com 3 sensores HC-SR04
//                 + Correção de trajetória para manter centro do corredor (desvia se <3cm de parede lateral)
//                 + Vira à direita só se espaço >20cm
// Driver de motores: L298P
// Configuração: Corredores de 40cm largura, carro de 20cm largura, sensores laterais a ~10cm de paredes no centro
// Movimentos: Alta velocidade (255), duração de 10ms, parada de 800ms
// Pinos extras: Incluídos para HuskyLens (8/9), OLED (A4/A5 I2C), LEDs (4-7), mesmo sem uso
// ===============================================

// --- LIGAÇÕES DOS SENSORES ---
#define TRIG_F A0  // Sensor Frente: Trigger
#define ECHO_F A1  // Sensor Frente: Echo
#define TRIG_L 2   // Sensor Esquerda: Trigger
#define ECHO_L 3   // Sensor Esquerda: Echo
#define TRIG_R A2  // Sensor Direita: Trigger
#define ECHO_R A3  // Sensor Direita: Echo

// --- LIGAÇÕES DOS MOTORES (L298P) ---
// Motor Direito: DIR (M1) = 12 (LOW = frente, HIGH = reverso), PWM (E1) = 10
// Motor Esquerdo: DIR (M2) = 13 (LOW = frente, HIGH = reverso), PWM (E2) = 11
#define RIGHT_DIR 12  // M1: Direção motor direito
#define RIGHT_PWM 10  // E1: PWM motor direito
#define LEFT_DIR 13   // M2: Direção motor esquerdo
#define LEFT_PWM 11   // E2: PWM motor esquerdo

// --- PINOS EXTRAS (MESMO SEM USO) ---
// HuskyLens (câmera): RX=8, TX=9 (SoftwareSerial)
#define HUSKY_RX 8
#define HUSKY_TX 9
// OLED: SDA=A4, SCL=A5 (I2C padrão)
// LEDs: Verde=4, Azul=5, Amarelo=6, Vermelho=7
#define LED_VERDE 4
#define LED_AZUL 5
#define LED_AMARELO 6
#define LED_VERMELHO 7

// --- CONFIGURAÇÕES ---
#define THRESHOLD 20       // Distância mínima para obstáculo frontal ou esquerda (cm)
#define RIGHT_THRESHOLD 20 // Distância mínima para virar à direita (cm) - mais estrito
#define MIN_SIDE_DIST 10    // Distância mínima lateral para correção (cm) - desvia se <3cm
#define TARGET_SIDE_DIST 10 // Distância alvo para sensores laterais (cm) - centro do corredor
#define SPEED 255          // Velocidade alta para todos os movimentos (0-255)
#define CORRECT_SPEED 180  // Velocidade reduzida para correções suaves (0-255)
#define MOVE_DURATION 150  // Duração dos movimentos (ms) - muito curto
#define STOP_DURATION 800  // Duração da parada após cada movimento (ms)

// --- Função que mede a distância de um sensor ---
long medirDistancia(int trigPin, int echoPin) {
  long duracao, distancia;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duracao = pulseIn(echoPin, HIGH, 30000); // Timeout de 30ms (~5m)
  distancia = duracao * 0.034 / 2;

  if (duracao == 0) return 200; // Se sem leitura, assume longe (livre)
  return distancia;
}

// --- Funções de controle dos motores (baseado em AI_CHALLENGER_MOVIMENTO.ino) ---
void moveForward(int leftSpeed, int rightSpeed) {
  digitalWrite(RIGHT_DIR, LOW);  // Direito: frente
  digitalWrite(LEFT_DIR, LOW);   // Esquerdo: frente
  analogWrite(RIGHT_PWM, rightSpeed);
  analogWrite(LEFT_PWM, leftSpeed);
  delay(MOVE_DURATION);
  stopMotors();
  delay(STOP_DURATION);
}

void turnLeft() {
  digitalWrite(LEFT_DIR, HIGH);  // Esquerdo: reverso (vira à esquerda)
  digitalWrite(RIGHT_DIR, LOW);  // Direito: frente
  analogWrite(LEFT_PWM, SPEED);
  analogWrite(RIGHT_PWM, SPEED);
  delay(MOVE_DURATION);
  stopMotors();
  delay(STOP_DURATION);
}

void turnRight() {
  digitalWrite(RIGHT_DIR, HIGH); // Direito: reverso (vira à direita)
  digitalWrite(LEFT_DIR, LOW);   // Esquerdo: frente
  analogWrite(RIGHT_PWM, SPEED);
  analogWrite(LEFT_PWM, SPEED);
  delay(MOVE_DURATION);
  stopMotors();
  delay(STOP_DURATION);
}

void turnBack() {  // U-turn (vira 180° - usa turnLeft por simplicidade)
  digitalWrite(LEFT_DIR, HIGH);  // Esquerdo: reverso
  digitalWrite(RIGHT_DIR, LOW);  // Direito: frente
  analogWrite(LEFT_PWM, SPEED);
  analogWrite(RIGHT_PWM, SPEED);
  delay(MOVE_DURATION);
  stopMotors();
  delay(STOP_DURATION);
}

// --- Função para parar os motores ---
void stopMotors() {
  analogWrite(RIGHT_PWM, 0);     // Desliga PWM direito
  analogWrite(LEFT_PWM, 0);      // Desliga PWM esquerdo
}

// -----------------------------------------------
void setup() {
  // Configura pinos dos sensores
  pinMode(TRIG_F, OUTPUT);
  pinMode(ECHO_F, INPUT);
  pinMode(TRIG_L, OUTPUT);
  pinMode(ECHO_L, INPUT);
  pinMode(TRIG_R, OUTPUT);
  pinMode(ECHO_R, INPUT);

  // Configura pinos dos motores (L298P)
  pinMode(RIGHT_DIR, OUTPUT);
  pinMode(RIGHT_PWM, OUTPUT);
  pinMode(LEFT_DIR, OUTPUT);
  pinMode(LEFT_PWM, OUTPUT);

  // Configura pinos extras (mesmo sem uso)
  pinMode(HUSKY_RX, INPUT);      // RX para HuskyLens
  pinMode(HUSKY_TX, OUTPUT);     // TX para HuskyLens
  // OLED usa A4/A5 por padrão no I2C, não precisa pinMode
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  // Inicia Serial para depuração
  Serial.begin(9600);
  Serial.println("=== CARRO ROBÔ LABIRINTO - REGRA DA DIREITA COM CORREÇÃO ===");
  Serial.println("Leituras em centímetros (cm)");
  Serial.println("---------------------------------");

  // Apaga LEDs iniciais (mesmo sem uso)
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AZUL, LOW);
  digitalWrite(LED_AMARELO, LOW);
  digitalWrite(LED_VERMELHO, LOW);
}

// -----------------------------------------------
void loop() {
  // Mede distâncias
  long distE = medirDistancia(TRIG_L, ECHO_L); // Esquerda
  long distF = medirDistancia(TRIG_F, ECHO_F); // Frente
  long distD = medirDistancia(TRIG_R, ECHO_R); // Direita

  // Mostra leituras no Serial
  Serial.print("Esq: "); Serial.print(distE); Serial.print(" cm | ");
  Serial.print("Frente: "); Serial.print(distF); Serial.print(" cm | ");
  Serial.print("Dir: "); Serial.print(distD); Serial.println(" cm");

  // Lógica: Regra da Mão Direita para Labirintos + Correção de Centro
  // Prioridade: Frente livre? Segue em frente (com correção lateral).
  // Senão, direita livre (>20cm)? Vira à direita.
  // Senão, esquerda livre? Vira à esquerda.
  // Senão, U-turn.
  if (distF > THRESHOLD) {
    // Frente livre: Segue em frente, mas corrige se perto de parede lateral
    if (distE < MIN_SIDE_DIST) {
      // Perto da esquerda (<3cm): Desvia ligeiramente à direita
      moveForward(CORRECT_SPEED, SPEED);
    } else if (distD < MIN_SIDE_DIST) {
      // Perto da direita (<3cm): Desvia ligeiramente à esquerda
      moveForward(SPEED, CORRECT_SPEED);
    } else if (abs(distE - distD) > 2) { // Diferença >2cm entre lados: corrige para centrar
      if (distE < distD) {
        // Mais perto da esquerda: Desvia ligeiramente à direita
        moveForward(CORRECT_SPEED, SPEED);
      } else {
        // Mais perto da direita: Desvia ligeiramente à esquerda
        moveForward(SPEED, CORRECT_SPEED);
      }
    } else {
      // Centrado (~10cm de cada lado): Avança reto
      moveForward(SPEED, SPEED);
    }
  } else if (distD > RIGHT_THRESHOLD) {
    // Direita livre (>20cm): Vira à direita
    turnRight();
  } else if (distE > THRESHOLD) {
    // Esquerda livre: Vira à esquerda
    turnLeft();
  } else {
    // Todos bloqueados: U-turn
    turnBack();
  }

  delay(100);  // Pequeno delay entre ciclos para evitar sobrecarga
}