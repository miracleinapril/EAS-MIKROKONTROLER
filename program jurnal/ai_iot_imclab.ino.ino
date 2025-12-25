#include <Arduino.h>

// ===== PIN iMCLab (sesuaikan kalau beda) =====
const int motor1Pin1 = 27;
const int motor1Pin2 = 26;
const int enable1Pin = 12;     // PWM enable
const int pin_rpm    = 13;     // input sensor RPM

// ===== PWM =====
const int freq = 30000;
const int resolution = 8;      // 0-255

// ===== RPM =====
volatile unsigned long rev = 0;
unsigned long last_rev_count = 0;
unsigned long last_time = 0;
float rpm_filtered = 0.0;

// ===== Control =====
int pwmValue = 0;
unsigned long lastSend = 0;

// Interrupt: hitung pulsa
void IRAM_ATTR countPulse() {
  rev++;
}

void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  pinMode(pin_rpm, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pin_rpm), countPulse, RISING);

  // PWM attach (ESP32 core 3.x)
  ledcAttach(enable1Pin, freq, resolution);
  ledcWrite(enable1Pin, 0);

  // set arah motor (coba ini dulu)
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);

  last_time = millis();

  // header CSV buat Python/AI
  Serial.println("PWM,RPM");
}

float readRPM() {
  unsigned long now = millis();
  unsigned long dt = now - last_time;

  if (dt >= 1000) {
    unsigned long current_rev;

    noInterrupts();
    current_rev = rev;
    interrupts();

    unsigned long pulses = current_rev - last_rev_count;
    last_rev_count = current_rev;
    last_time = now;

    const int holes = 2; // ubah sesuai encoder kamu (misal 2 lubang)
    float rotations = (float)pulses / holes;
    float rpm = (rotations * 60000.0) / (float)dt;

    // smoothing biar stabil
    rpm_filtered = 0.7f * rpm_filtered + 0.3f * rpm;
  }

  return rpm_filtered;
}

void loop() {
  // ===== AUTO PWM (buat ambil data) =====
  pwmValue += 20;
  if (pwmValue > 255) pwmValue = 80; // mulai dari 80 biar motor kuat muter

  // kirim PWM ke motor
  ledcWrite(enable1Pin, pwmValue);

  // baca rpm asli
  float rpm = readRPM();

  // kirim data tiap 1 detik (CSV)
  if (millis() - lastSend > 1000) {
    Serial.print(pwmValue);
    Serial.print(",");
    Serial.println(rpm);
    lastSend = millis();
  }

  delay(200); // biar nggak terlalu cepat naik
}
