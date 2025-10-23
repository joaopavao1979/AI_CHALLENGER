// ===============================================
// 🚗 LABIRINTO – REGRA DA DIREITA + CENTRAGEM (3x HC-SR04)
// Projeto: AI CHALLENGER – AzoresBot
// Driver: L298P  (M1: DIR=12 PWM=10) (M2: DIR=13 PWM=11)
// Sensores: Frente(A0/A1), Esquerda(2/3), Direita(A2/A3)
// Notas: corredores ~40 cm; alvo ~10 cm de cada parede
// ===============================================

// ---------- SENSORES ----------
#define TRIG_F A0
#define ECHO_F A1
#define TRIG_L 2
#define ECHO_L 3
#define TRIG_R A2
#define ECHO_R A3

// ---------- MOTORES (L298P) ----------
#define RIGHT_DIR 12
#define RIGHT_PWM 10
#define LEFT_DIR  13
#define LEFT_PWM  11

// ---------- RESERVADOS (sem uso agora) ----------
#define HUSKY_RX 8
#define HUSKY_TX 9
#define LED_VERDE 4
#define LED_AZUL 5
#define LED_AMARELO 6
#define LED_VERMELHO 7
// OLED I2C: A4/A5

// ---------- CONFIG ----------
#define FRONT_CLEAR      20   // Frente livre se > 20 cm
#define RIGHT_CLEAR      22   // Direita precisa estar um pouco mais folgada
#define LEFT_CLEAR       20
#define MIN_SIDE_DIST     7   // Corrige se ficar < ~7 cm da parede
#define TARGET_SIDE_DIST 10   // Alvo ~10 cm de cada lado

#define BASE_SPEED      200   // Velocidade base (0–255)
#define MAX_DELTA        70   // Máx. correção diferencial aplicada aos PWMs
#define KP               4    // Ganho proporcional simples (ajusta em testes)

#define STEP_MS         120   // Passo curto a direito
#define STOP_MS          60   // Paragem breve entre passos
#define TURN_90_MS      320   // Ajusta ao teu chassis (≈90°)
#define TURN_180_MS     640   // ≈180° (podes duplicar do 90)

#define PULSE_TIMEOUT 30000UL // us (~5 m)

// ---------- UTILS ----------
long medirCru(unsigned trigPin, unsigned echoPin) {
  digitalWrite(trigPin, LOW);  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  unsigned long dur = pulseIn(echoPin, HIGH, PULSE_TIMEOUT);
  if (dur == 0) return 200; // sem eco → assume longe
  return (long)(dur * 0.034 / 2.0);
}

// Mediana de 3 leituras para estabilidade
long medirCM(unsigned trigPin, unsigned echoPin) {
  long a = medirCru(trigPin, echoPin);
  long b = medirCru(trigPin, echoPin);
  long c = medirCru(trigPin, echoPin);
  // ordena 3 valores rapidamente
  if (a > b) { long t=a; a=b; b=t; }
  if (b > c) { long t=b; b=c; c=t; }
  if (a > b) { long t=a; a=b; b=t; }
  return b; // mediana
}

void stopMotors() {
  analogWrite(RIGHT_PWM, 0);
  analogWrite(LEFT_PWM, 0);
}

void drive(int leftSpd, int rightSpd, int ms) {
  // Direção: LOW = frente; HIGH = marcha-atrás (ajusta se o teu wiring for o inverso)
  digitalWrite(LEFT_DIR,  (leftSpd  >= 0) ? LOW : HIGH);
  digitalWrite(RIGHT_DIR, (rightSpd >= 0) ? LOW : HIGH);
  leftSpd  = constrain(abs(leftSpd),  0, 255);
  rightSpd = constrain(abs(rightSpd), 0, 255);
  analogWrite(LEFT_PWM,  leftSpd);
  analogWrite(RIGHT_PWM, rightSpd);
  delay(ms);
}

void forwardStep(int leftSpd, int rightSpd) {
  drive(leftSpd, rightSpd, STEP_MS);
  stopMotors();
  delay(STOP_MS);
}

void turnRight90() {
  // Pivot: roda direita para trás, esquerda para a frente
  drive(+BASE_SPEED, -BASE_SPEED, TURN_90_MS);
  stopMotors(); delay(STOP_MS);
}

void turnLeft90() {
  drive(-BASE_SPEED, +BASE_SPEED, TURN_90_MS);
  stopMotors(); delay(STOP_MS);
}

void turnBack180() {
  drive(-BASE_SPEED, +BASE_SPEED, TURN_180_MS);
  stopMotors(); delay(STOP_MS);
}

// ---------- SETUP ----------
void setup() {
  pinMode(TRIG_F, OUTPUT); pinMode(ECHO_F, INPUT);
  pinMode(TRIG_L, OUTPUT); pinMode(ECHO_L, INPUT);
  pinMode(TRIG_R, OUTPUT); pinMode(ECHO_R, INPUT);

  pinMode(RIGHT_DIR, OUTPUT); pinMode(RIGHT_PWM, OUTPUT);
  pinMode(LEFT_DIR,  OUTPUT); pinMode(LEFT_PWM,  OUTPUT);

  pinMode(HUSKY_RX, INPUT);
  pinMode(HUSKY_TX, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  Serial.begin(9600);
  Serial.println(F("=== LABIRINTO: Regra da Direita + Centragem ==="));
  digitalWrite(LED_VERDE, HIGH); // alive
}

// ---------- LOOP ----------
void loop() {
  long distE = medirCM(TRIG_L, ECHO_L);
  long distF = medirCM(TRIG_F, ECHO_F);
  long distD = medirCM(TRIG_R, ECHO_R);

  Serial.print(F("Esq: ")); Serial.print(distE);
  Serial.print(F(" | Frente: ")); Serial.print(distF);
  Serial.print(F(" | Dir: ")); Serial.println(distD);

  // 1) Se frente livre → avança corrigindo para o centro
  if (distF > FRONT_CLEAR) {
    // Fica atento se estiver a “roçar” numa parede
    if (distE < MIN_SIDE_DIST) {
      // colado à esquerda → puxa ligeiro para direita
      forwardStep(BASE_SPEED - 0, BASE_SPEED + 40);
      return;
    }
    if (distD < MIN_SIDE_DIST) {
      // colado à direita → puxa ligeiro para esquerda
      forwardStep(BASE_SPEED + 40, BASE_SPEED - 0);
      return;
    }

    // Correção proporcional simples para centragem
    // erro > 0 => mais espaço à direita => virar um pouco à direita (reduz esquerdo, aumenta direito)
    long erro = distD - distE;               // alvo ideal ~0
    long delta = constrain(erro * KP, -MAX_DELTA, +MAX_DELTA);
    int l = constrain(BASE_SPEED - delta, 0, 255);
    int r = constrain(BASE_SPEED + delta, 0, 255);
    forwardStep(l, r);
    return;
  }

  // 2) Sem frente: tenta direita (regra da mão direita)
  if (distD > RIGHT_CLEAR) {
    turnRight90();
    return;
  }

  // 3) Direita fechada mas esquerda livre
  if (distE > LEFT_CLEAR) {
    turnLeft90();
    return;
  }

  // 4) Cul-de-sac: meia-volta
  turnBack180();
}
