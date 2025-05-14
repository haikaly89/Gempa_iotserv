#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// Pin GPIO
const int buzzerPin = 13;
const int ledPin = 12;

const float threshold = 1.18;  // Ambang batas guncangan (dalam g)
const unsigned long detectionDuration = 250; // dalam ms (2 detik)
unsigned long startTime = 0;
bool aboveThreshold = false;

void setup() {
  Serial.begin(115200);

  // Inisialisasi I2C dan sensor
  Wire.begin(21, 22); // SDA, SCL
  mpu.initialize();

  // Cek koneksi MPU6050
  if (!mpu.testConnection()) {
    Serial.println("❌ MPU6050 tidak terdeteksi!");
    while (1);
  }
  Serial.println("✅ MPU6050 siap.");

  // Setup pin output
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  digitalWrite(ledPin, LOW);
}

void loop() {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  // Konversi ke satuan g
  float x = ax / 16384.0;
  float y = ay / 16384.0;
  float z = az / 16384.0;

  float a_total = sqrt(x * x + y * y + z * z);

  Serial.print("a_total: ");
  Serial.println(a_total);

  if (a_total > threshold) {
    if (!aboveThreshold) {
      startTime = millis();
      aboveThreshold = true;
    } else if (millis() - startTime >= detectionDuration) {
      Serial.println("⚠️ Gempa terdeteksi!");

      // Aktifkan alarm
      digitalWrite(buzzerPin, HIGH);
      digitalWrite(ledPin, HIGH);
      delay(2000);  // Alarm selama 2 detik
      digitalWrite(buzzerPin, LOW);
      digitalWrite(ledPin, LOW);

      // Reset
      aboveThreshold = false;
    }
  } else {
    aboveThreshold = false;
  }

  delay(100);
}
