#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#include "ModeManager.h"
#include "EmoteMode.h"
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

// เรียก onEnter() ของโหมดที่ "กำลังจะเข้า"
void enterMode(AppMode m) {
  switch (m) {
    case MODE_EMOTE: EmoteMode::onEnter(); break;
    case MODE_ALARM: AlarmMode::onEnter(); break;
    case MODE_MUSIC: MusicMode::onEnter(); break;
  }
}

// เรียก onExit() ของโหมดที่ "กำลังจะออก"
void exitMode(AppMode m) {
  switch (m) {
    case MODE_EMOTE: EmoteMode::onExit(); break;
    case MODE_ALARM: AlarmMode::onExit(); break;
    case MODE_MUSIC: MusicMode::onExit(); break;
  }
}

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

  enterMode(modeManager.mode());   // เริ่มที่โหมด default (Emote)
}

void loop() {
  unsigned long now = millis();

  // อ่านอุณหภูมิทุก 2 วินาที
  if (now - lastDHTReadTime >= 2000UL) {
    lastDHTReadTime = now;
    float t = dht.readTemperature();
    if (!isnan(t)) currentTemp = t;
  }

  bool pressed = modeManager.buttonPressed();
  AppMode mode = modeManager.mode();

  if (pressed) {
    if (mode == MODE_MUSIC) {
      // อยู่ในโหมด Music: ปุ่มถูกเมนูของ Music ดักไปใช้เอง
      // ขอสลับโหมดจริงก็ต่อเมื่อเคอร์เซอร์เดินไปถึงช่อง "Mode" เท่านั้น
      bool wantsModeSwitch = MusicMode::onButtonPress();
      if (wantsModeSwitch) {
        exitMode(mode);
        modeManager.advanceMode();
        enterMode(modeManager.mode());
      }
    } else {
      // โหมดอื่น: กดปุ่มแล้วสลับโหมดทันทีตามปกติ
      exitMode(mode);
      modeManager.advanceMode();
      enterMode(modeManager.mode());
    }
  }

  switch (modeManager.mode()) {
    case MODE_EMOTE:
      EmoteMode::update(currentTemp);
      EmoteMode::draw(display, currentTemp, OLED_X_OFFSET);
      break;

    case MODE_ALARM:
      AlarmMode::update(currentTemp);
      AlarmMode::draw(display, OLED_X_OFFSET);
      break;

    case MODE_MUSIC:
      MusicMode::update(currentTemp);
      MusicMode::draw(display, OLED_X_OFFSET);
      break;
  }
}