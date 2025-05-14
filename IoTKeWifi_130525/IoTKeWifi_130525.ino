#include <Wire.h>
#include <MPU6050.h>
#include <WiFi.h>
#include <HTTPClient.h>

MPU6050 mpu;

// Ganti dengan kredensial WiFi kamu
const char* ssid = "Pioti";
const char* password = "0000aaaa";

// Ganti dengan IP lokal atau publik Flask server kamu
const String serverUrl = "http://192.168.248.88:5000/data";

const int buzzerPin = 13;
const int ledPin = 12;

const float threshold = 1.18;
const unsigned long detectionDuration = 250;
unsigned long startTime = 0;
bool aboveThreshold = false;

float estimateRichter(float a_total) {
  if (a_total <= 1.0) return 2.0;
  else if (a_total <= 1.5) return 3.0;
  else if (a_total <= 2.0) return 4.0;
  else if (a_total <= 2.5) return 5.0;
  else if (a_total <= 3.0) return 6.0;
  else if (a_total <= 4.0) return 7.0;
  else return 8.0;
}

void getMMIInfo(float richter, String &mmi, String &gol) {
  if (richter <= 2.0) { mmi = "I"; gol = "Instrumental"; }
  else if (richter <= 3.0) { mmi = "II"; gol = "Sangat lemah"; }
  else if (richter <= 4.0) { mmi = "III"; gol = "Agak lemah"; }
  else if (richter == 4.0) { mmi = "IV"; gol = "Lemah"; }
  else if (richter <= 5.0) { mmi = "V"; gol = "Agak kuat"; }
  else if (richter <= 6.0) { mmi = "VI"; gol = "Kuat"; }
  else if (richter == 6.0) { mmi = "VII"; gol = "Sangat kuat"; }
  else if (richter <= 7.0) { mmi = "VIII"; gol = "Merusak"; }
  else if (richter == 7.0) { mmi = "IX"; gol = "Hebat"; }
  else if (richter <= 8.0) { mmi = "X"; gol = "Sangat Hebat"; }
  else if (richter == 8.0) { mmi = "XI"; gol = "Ekstrim"; }
  else { mmi = "XII"; gol = "Sangat Ekstrim"; }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("❌ MPU6050 tidak terdeteksi!");
    while (1);
  }
  Serial.println("✅ MPU6050 siap.");

  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  digitalWrite(ledPin, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Tersambung!");
}

void loop() {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  float x = ax / 16384.0;
  float y = ay / 16384.0;
  float z = az / 16384.0;

  float a_total = sqrt(x * x + y * y + z * z);

  if (a_total > threshold) {
    if (!aboveThreshold) {
      startTime = millis();
      aboveThreshold = true;
    } else if (millis() - startTime >= detectionDuration) {
      float richter = estimateRichter(a_total);
      String mmi, gol;
      getMMIInfo(richter, mmi, gol);

      Serial.println("⚠️ Gempa Terdeteksi!");
      Serial.println("Mengirim data...");

      String payload = "{\"richter\": " + String(richter, 1) +
                       ", \"mmi\": \"" + mmi +
                       "\", \"golongan\": \"" + gol + "\"}";

      HTTPClient http;
      http.begin(serverUrl);
      http.addHeader("Content-Type", "application/json");
      int httpResponseCode = http.POST(payload);
      if (httpResponseCode > 0) {
        Serial.print("Response: ");
        Serial.println(httpResponseCode);
        Serial.println(http.getString());
      } else {
        Serial.print("Error POST: ");
        Serial.println(http.errorToString(httpResponseCode));
      }
      http.end();

      digitalWrite(buzzerPin, HIGH);
      digitalWrite(ledPin, HIGH);
      delay(2000);
      digitalWrite(buzzerPin, LOW);
      digitalWrite(ledPin, LOW);

      aboveThreshold = false;
    }
  } else {
    aboveThreshold = false;
  }

  delay(100);
}
