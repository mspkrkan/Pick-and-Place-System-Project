#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "id"
#define BLYNK_TEMPLATE_NAME "Pick and Place ESP32"
#define BLYNK_AUTH_TOKEN "auth"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char auth[] = "auth";
char ssid[] = "Wİ-Fİ-SSID";
char pass[] = "WİFİ PASSWORD";

/***********************************************************
 * PIN TANIMLARI
 ***********************************************************/
const int X_STEP_PIN = 26;
const int X_DIR_PIN  = 27;
const int X_LIMIT_PIN = 22;

const int Z_STEP_PIN = 14;
const int Z_DIR_PIN  = 12;
const int Z_LIMIT_PIN = 21;

const int POMPA_IN1 = 25;
const int POMPA_IN2 = 33;
const int VALF_IN3  = 32;
const int VALF_IN4  = 18;

/***********************************************************
 * AYARLAR
 ***********************************************************/
#define STEP_DELAY 100
#define MAX_STEP   19500

long xPos = 0;
long zPos = 0;
bool homingDone = false;

void guvenliKapat() {
  vakumKapat();
}

/***********************************************************
 * STEP FONKSİYONU
 ***********************************************************/
void stepAt(int pin) {
  digitalWrite(pin, HIGH);
  delayMicroseconds(STEP_DELAY);
  digitalWrite(pin, LOW);
  delayMicroseconds(STEP_DELAY);
}

/***********************************************************
 * DEBOUNCE
 ***********************************************************/
bool switchBasiliMi(int sw) {
  unsigned long t = millis();
  while (millis() - t < 20) {
    if (digitalRead(sw) != LOW) return false;
  }
  return true;
}

/***********************************************************
 * HOMING
 ***********************************************************/
void homingX() {
  digitalWrite(X_DIR_PIN, LOW);
  while (!switchBasiliMi(X_LIMIT_PIN)) {
    stepAt(X_STEP_PIN);
  }
  xPos = 0;
}

void homingZ() {
  digitalWrite(Z_DIR_PIN, LOW);
  while (!switchBasiliMi(Z_LIMIT_PIN)) {
    stepAt(Z_STEP_PIN);
  }
  zPos = 0;
}

/***********************************************************
 * HEDEF POZİSYONA GİTME
 ***********************************************************/
void moveAxis(long &current, long target, int stepPin, int dirPin) {
  if (target == current) return;

  digitalWrite(dirPin, target > current ? HIGH : LOW);
  long steps = abs(target - current);

  for (long i = 0; i < steps; i++) stepAt(stepPin);
  current = target;
}

/***********************************************************
 * VAKUM
 ***********************************************************/
void vakumAc() {
  digitalWrite(VALF_IN3, LOW);
  digitalWrite(VALF_IN4, LOW);
  digitalWrite(POMPA_IN1, HIGH);
  digitalWrite(POMPA_IN2, LOW);
}

void vakumKapat() {
  digitalWrite(POMPA_IN1, LOW);
  digitalWrite(POMPA_IN2, LOW);
  digitalWrite(VALF_IN3, HIGH);
  digitalWrite(VALF_IN4, LOW);
  delay(800);
  digitalWrite(VALF_IN3, LOW);
  digitalWrite(VALF_IN4, LOW);
}

/***********************************************************
 * BLYNK KONTROLLERİ
 ***********************************************************/
BLYNK_WRITE(V0) {   // HOMING
  if (param.asInt()) {
    homingDone = false;
    homingX();
    delay(200);
    homingZ();
    homingDone = true;
  }
}

BLYNK_WRITE(V1) {   // X AXIS %
  if (!homingDone) return;
  long target = map(param.asInt(), 0, 100, 0, MAX_STEP);
  moveAxis(xPos, target, X_STEP_PIN, X_DIR_PIN);
}

BLYNK_WRITE(V2) {   // Z AXIS %
  if (!homingDone) return;
  long target = map(param.asInt(), 0, 100, 0, MAX_STEP);
  moveAxis(zPos, target, Z_STEP_PIN, Z_DIR_PIN);
}

BLYNK_WRITE(V3) {   // VACUUM
  if (param.asInt()) vakumAc();
  else vakumKapat();
}

/***********************************************************
 * SETUP & LOOP
 ***********************************************************/
void setup() {
  Serial.begin(115200);

  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_LIMIT_PIN, INPUT_PULLUP);

  pinMode(Z_STEP_PIN, OUTPUT);
  pinMode(Z_DIR_PIN, OUTPUT);
  pinMode(Z_LIMIT_PIN, INPUT_PULLUP);

  pinMode(POMPA_IN1, OUTPUT);
  pinMode(POMPA_IN2, OUTPUT);
  pinMode(VALF_IN3, OUTPUT);
  pinMode(VALF_IN4, OUTPUT);

  Blynk.begin(auth, ssid, pass);
}

BLYNK_WRITE(V4) {
  if (param.asInt()) {
    vakumKapat();
    delay(100);
    ESP.restart();
  }
}

void loop() {
  Blynk.run();
}
