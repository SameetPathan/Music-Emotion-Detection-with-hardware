#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// ─── WiFi & Firebase Config ───────────────────────────────────
#define WIFI_SSID     "Airtel_Jarvis"
#define WIFI_PASSWORD "Sameet786"
#define FIREBASE_HOST "jarvis-systems-commons-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "fsclmh960lKOv4CuIbyDZQ2QzuF0g2jjGvH1hh4Y" 

// ─── LED Matrix Config ────────────────────────────────────────
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES   4
#define CS_PIN        D8
#define DATA_PIN      D7
#define CLK_PIN       D5

// ─── Firebase & Matrix Objects ────────────────────────────────
FirebaseData fbData;
FirebaseConfig fbConfig;
FirebaseAuth fbAuth;

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

int lastEmotion = -1;

// ─── Light only the section matching emotion (1–8) ────────────
void showSection(int section) {
  if (section < 1 || section > 8) {
    mx.clear();  // Invalid value → turn everything off
    return;
  }

  mx.clear();

  int startCol = (section - 1) * 4;  // e.g. emotion=2 → col 4
  int endCol   = startCol + 3;        // e.g. emotion=2 → col 7

  for (int col = startCol; col <= endCol; col++) {
    mx.setColumn(col, 0xFF);  // All 8 rows ON for this column
  }

  Serial.printf("Emotion %d → Columns %d to %d lit\n", section, startCol, endCol);
}

void setup() {
  Serial.begin(115200);

  // ── Init LED Matrix ──
  mx.begin();
  mx.clear();
  mx.control(MD_MAX72XX::INTENSITY, 5);

  // ── Connect WiFi ──
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected! IP: " + WiFi.localIP().toString());

  // ── Connect Firebase ──
  fbConfig.host = FIREBASE_HOST;
  fbConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&fbConfig, &fbAuth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase Connected!");
}

void loop() {
  // Read /emotion from Firebase
  if (Firebase.getInt(fbData, "/emotion")) {
    int emotion = fbData.intData();

    if (emotion != lastEmotion) {
      lastEmotion = emotion;
      Serial.println("Emotion changed: " + String(emotion));
      showSection(emotion);
    }
  } else {
    Serial.println("Firebase error: " + fbData.errorReason());
  }

  delay(1000);  // Check every second
}