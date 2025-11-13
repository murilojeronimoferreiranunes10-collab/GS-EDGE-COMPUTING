#include <Servo.h>

// === PINOS ===
#define TRIG 6
#define ECHO 7
#define LDR A0
#define LM35 A2
#define BUZZER A3
#define RED_PIN 9
#define GREEN_PIN 10
#define BLUE_PIN 11
#define SERVO_PIN 3

Servo servoMotor;

// === VARIÁVEIS GLOBAIS ===
unsigned long startTime;
int ldrMin = 1023;
int ldrMax = 0;
bool calibrated = false;

void setup() {
  Serial.begin(9600);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(LDR, INPUT);
  pinMode(LM35, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  
  servoMotor.attach(SERVO_PIN);

  Serial.println("=== SMARTDESK 2050 ===");
  Serial.println("Calibrando sensor de luz (6 segundos)...");
  startTime = millis();
}

float medirDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  float duracao = pulseIn(ECHO, HIGH);
  float distancia = duracao * 0.034 / 2;
  if (distancia <= 0 || distancia > 400) distancia = 0;
  return distancia;
}

void setRGB(int r, int g, int b) {
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}

// === Função para ler e converter o LDR ===
int lerLuzPorcentagem() {
  int raw = analogRead(LDR);

  // Durante a calibração inicial (6 s)
  if (!calibrated) {
    if (raw < ldrMin) ldrMin = raw;
    if (raw > ldrMax) ldrMax = raw;
    if (millis() - startTime > 6000) {
      calibrated = true;
      if (ldrMax == ldrMin) ldrMax = ldrMin + 1;
      Serial.print("Calibracao concluida. Min=");
      Serial.print(ldrMin);
      Serial.print("  Max=");
      Serial.println(ldrMax);
    }
  }

  // Mapeia para 0–100%
  int pct = map(raw, ldrMin, ldrMax, 0, 100);
  pct = constrain(pct, 0, 100);

  // Agrupa em 0, 50 ou 100
  if (pct < 30) pct = 0;
  else if (pct < 70) pct = 50;
  else pct = 100;

  return pct;
}

// === LOOP PRINCIPAL ===
void loop() {
  // Lê sensores
  float distancia = medirDistancia();
  int luzPct = lerLuzPorcentagem();

  int leituraLM35 = analogRead(LM35);
  float temperatura = (leituraLM35 * 5.0 * 100.0) / 1023.0;

  // Servo — abre se alguém perto (< 20 cm)
  if (distancia > 0 && distancia < 20) servoMotor.write(90);
  else servoMotor.write(0);

  // RGB e buzzer
  if (temperatura > 30) {
    setRGB(255, 0, 0);  // Vermelho — quente
    digitalWrite(BUZZER, HIGH);
  } else if (luzPct == 0) {
    setRGB(0, 0, 255);  // Azul — escuro
    digitalWrite(BUZZER, LOW);
  } else if (luzPct == 50) {
    setRGB(255, 255, 0); // Amarelo — média luz
    digitalWrite(BUZZER, LOW);
  } else {
    setRGB(0, 255, 0);  // Verde — luz alta
    digitalWrite(BUZZER, LOW);
  }

  // Serial monitor
  Serial.print("Dist: ");
  Serial.print(distancia);
  Serial.print(" cm  |  Temp: ");
  Serial.print(temperatura, 1);
  Serial.print(" C  |  Luz: ");
  Serial.print(luzPct);
  Serial.println("%");

  delay(700);
}
