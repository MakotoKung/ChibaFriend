#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#include "modeManager.h"
#include "Emotemode.h"
#include "AlarmMode.h"
#include "MusicMode.h"

// ---------------- OLED SSD1306 ขนาด 64x32 ----------------
#define SCREEN_I2C_ADDR 0x3C
#define OLED_RST_PIN    -1
#define OLED_X_OFFSET   32   // จอ 64x32 เห็นแค่คอลัมน์ 32-95 ของ RAM 128 คอลัมน์

Adafruit_SSD1306 display(128, 32, &Wire, OLED_RST_PIN);

// ---------------- DHT11 ----------------
#define DHTPIN  4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ---------------- ปุ่ม (active-low) ----------------
#define BUTTON_PIN 15
ModeManager modeManager(BUTTON_PIN);

float currentTemp = 25.0f;
unsigned long lastDHTReadTime = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  dht.begin();
  modeManager.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.display();

  float t = dht.readTemperature();
  if (!isnan(t)) currentTemp = t;

  EmoteMode::onEnter();   // เริ่มที่โหมด default
}

void loop() {
  unsigned long now = millis();

  // อ่านอุณหภูมิทุก 2 วินาที
  if (now - lastDHTReadTime >= 2000UL) {
    lastDHTReadTime = now;
    float t = dht.readTemperature();
    if (!isnan(t)) currentTemp = t;
  }

  // เช็คปุ่ม -> ถ้ามีการกด modeManager จะเปลี่ยนโหมดให้เอง
  bool modeChanged = modeManager.update();

  switch (modeManager.mode()) {
    case MODE_EMOTE:
      if (modeChanged) EmoteMode::onEnter();
      EmoteMode::update(currentTemp);
      EmoteMode::draw(display, currentTemp, OLED_X_OFFSET);
      break;

    case MODE_ALARM:
      if (modeChanged) AlarmMode::onEnter();
      AlarmMode::update(currentTemp);
      AlarmMode::draw(display, OLED_X_OFFSET);
      break;

    case MODE_MUSIC:
      if (modeChanged) MusicMode::onEnter();
      MusicMode::update(currentTemp);
      MusicMode::draw(display, OLED_X_OFFSET);
      break;
  }
}