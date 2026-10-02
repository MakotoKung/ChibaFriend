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

// ---------------- ปุ่มหลัก (active-low) ----------------
// หมายเหตุ: GPIO15 เป็น strapping pin ของ ESP32 ต้องเป็น HIGH ตอนบูต
// เพราะปุ่มต่อแบบ active-low พร้อม INPUT_PULLUP (นิ่ง = HIGH) จึงไม่กระทบการบูตตามปกติ
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

  // ---------------- ปุ่มหลัก (PIN15) ----------------
  // แต่ละโหมดตัดสินใจเองว่า "กดแล้วจะสลับ AppMode จริงไหม"
  // Emote: สลับทันทีทุกครั้งที่กด (ไม่มีหน้าจอย่อยของตัวเอง)
  // Alarm/Music: มีหน้าจอ/เมนูของตัวเอง กดแล้วเลื่อนภายในโหมดก่อน สลับโหมดจริงเมื่อเลื่อนเกิน index สุดท้าย
  bool pressed = modeManager.buttonPressed();
  AppMode mode = modeManager.mode();

  if (pressed) {
    bool wantsModeSwitch;
    switch (mode) {
      case MODE_ALARM: wantsModeSwitch = AlarmMode::onMainButtonPress(); break;
      case MODE_MUSIC: wantsModeSwitch = MusicMode::onButtonPress();     break;
      default:         wantsModeSwitch = true;                          break;
    }
    if (wantsModeSwitch) {
      exitMode(mode);
      modeManager.advanceMode();
      enterMode(modeManager.mode());
    }
  }

  // ---------------- Alarm ทำงานเบื้องหลังตลอดเวลา ไม่ผูกกับโหมดปัจจุบัน ----------------
  // - นับเวลาถอยหลังและเริ่มกริ่งเมื่อครบ แม้ไม่ได้อยู่หน้าจอ Alarm
  // - ปุ่ม OK ปิดเสียงกริ่งได้จากทุกโหมด (ตั้ง/ยกเลิก timer ใหม่ได้เฉพาะตอนอยู่หน้าจอ Alarm)
  AlarmMode::update(currentTemp);
  AlarmMode::checkOkButton(modeManager.mode() == MODE_ALARM);

  // ---------------- วาดเฉพาะหน้าจอของโหมดปัจจุบัน ----------------
  switch (modeManager.mode()) {
    case MODE_EMOTE:
      EmoteMode::update(currentTemp);
      EmoteMode::draw(display, currentTemp, OLED_X_OFFSET);
      break;

    case MODE_ALARM:
      AlarmMode::draw(display, OLED_X_OFFSET);   // update() ถูกเรียกไปแล้วด้านบน
      break;

    case MODE_MUSIC:
      MusicMode::update(currentTemp);
      MusicMode::draw(display, OLED_X_OFFSET);
      break;
  }
}