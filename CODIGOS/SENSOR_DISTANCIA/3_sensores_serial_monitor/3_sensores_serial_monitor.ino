// ===============================================
// 🚗 TESTE DE SENSORES ULTRASSÓNICOS (HC-SR04)
// Projeto: AI CHALLENGER - AzoresBot
// Versão didática para alunos
// Autor: João Pavão
// ===============================================

// --- LIGAÇÕES DOS SENSORES ---
// Sensor da FRENTE : TRIG = A0 | ECHO = A1
// Sensor da ESQUERDA : TRIG = 2 | ECHO = 3
// Sensor da DIREITA : TRIG = A2 | ECHO = A3

#define TRIG_F A0
#define ECHO_F A1
#define TRIG_L 2
#define ECHO_L 3
#define TRIG_R A2
#define ECHO_R A3

// --- Função que mede a distância de um sensor ---
// Recebe os pinos TRIG e ECHO e devolve a distância em cm
long medirDistancia(int trigPin, int echoPin) {
  long duracao, distancia;

  // Garante que o pino TRIG começa em LOW
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Envia um pulso de 10 microssegundos
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Mede o tempo do eco em microssegundos
  duracao = pulseIn(echoPin, HIGH, 30000); // timeout 30 ms (≈ 5 m)

  // Converte tempo em distância (velocidade do som = 0.034 cm/µs)
  distancia = duracao * 0.034 / 2;

  // Se não houver leitura, devolve -1
  if (duracao == 0) return -1;

  return distancia;
}

// -----------------------------------------------
void setup() {
  // Configura pinos dos 3 sensores
  pinMode(TRIG_F, OUTPUT);
  pinMode(ECHO_F, INPUT);

  pinMode(TRIG_L, OUTPUT);
  pinMode(ECHO_L, INPUT);

  pinMode(TRIG_R, OUTPUT);
  pinMode(ECHO_R, INPUT);

  // Inicia comunicação serial
  Serial.begin(9600);
  Serial.println("=== TESTE DOS 3 SENSORES ULTRASSÓNICOS ===");
  Serial.println("Leituras em centímetros (cm)");
  Serial.println("-------------------------------------------");
}

// -----------------------------------------------
void loop() {
  // Mede a distância de cada sensor
  long distE = medirDistancia(TRIG_L, ECHO_L);  // esquerda
  long distF = medirDistancia(TRIG_F, ECHO_F);  // frente
  long distD = medirDistancia(TRIG_R, ECHO_R);  // direita

  // Mostra resultados no monitor serial
  Serial.print("Esq: ");
  if (distE == -1) Serial.print("----");
  else Serial.print(distE);
  Serial.print(" cm   |   ");

  Serial.print("Frente: ");
  if (distF == -1) Serial.print("----");
  else Serial.print(distF);
  Serial.print(" cm   |   ");

  Serial.print("Dir: ");
  if (distD == -1) Serial.print("----");
  else Serial.print(distD);
  Serial.println(" cm");

  delay(500); // espera meio segundo entre leituras
}