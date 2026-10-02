#pragma once
#include <Adafruit_SSD1306.h>
#include "Emote.h"

// buzzer ตัวเดียวกับที่ MusicMode ใช้ (ขา 25) - ประกาศแยกในไฟล์นี้ตามที่ขอ
// (ชื่อ/ค่าตรงกัน define ซ้ำแบบค่าเดียวกันไม่มีปัญหา ไม่ต้องแก้ MusicMode.h)
#define BUZZER_PIN 25
#define EMOTE_BEEP_FREQ 1200   // ความถี่เสียงบี๊บตอนอารมณ์เปลี่ยน (Hz)
#define EMOTE_BEEP_MS   60     // ความยาวเสียงบี๊บ (ms) - สั้น ไม่บล็อกลูป

// LED บอกสถานะอารมณ์: เหลือง=เย็น, เขียว=ปกติ, แดง=ร้อน
#define LED_YELLOW_PIN 14   // COLD
#define LED_GREEN_PIN  12   // NORMAL
#define LED_RED_PIN    13   // HOT

// ระยะห่างของ bitmap จากขอบซ้ายของพื้นที่แสดงผลจริง (หลังชดเชย offset แล้ว)
namespace EmoteMode {

inline EmoteType currentType = EMOTE_NORMAL;
inline uint8_t currentFrame = 0;
inline unsigned long lastFrameTime = 0;

// hysteresis 0.5 องศา กันภาพสลับไปมาตรงเส้นแบ่ง
inline EmoteType pickEmote(float t, EmoteType cur) {
  const float H = 0.5f;
  switch (cur) {
    case EMOTE_COLD: return (t > 27.0f + H) ? EMOTE_NORMAL : EMOTE_COLD;
    case EMOTE_HOT:  return (t < 28.5f - H) ? EMOTE_NORMAL : EMOTE_HOT;
    default:
      if (t <= 27.0f - H) return EMOTE_COLD;
      if (t >  28.5f + H) return EMOTE_HOT;
      return EMOTE_NORMAL;
  }
}

// ติด LED ดวงที่ตรงกับอารมณ์ปัจจุบัน แล้วดับอีก 2 ดวงเสมอ (กันไฟค้างจากอารมณ์ก่อนหน้า)
inline void setLeds(EmoteType type) {
  digitalWrite(LED_YELLOW_PIN, (type == EMOTE_COLD)   ? HIGH : LOW);
  digitalWrite(LED_GREEN_PIN,  (type == EMOTE_NORMAL) ? HIGH : LOW);
  digitalWrite(LED_RED_PIN,    (type == EMOTE_HOT)    ? HIGH : LOW);
}

inline void drawBitmapScaled(Adafruit_SSD1306 &display, int x, int y,
                              const uint8_t *bmp, int scale) {
  for (int r = 0; r < BMP_SIZE; r++) {
    uint8_t row = pgm_read_byte(bmp + r);
    for (int c = 0; c < BMP_SIZE; c++) {
      if (row & (0x80 >> c)) {
        display.fillRect(x + c * scale, y + r * scale, scale, scale, SSD1306_WHITE);
      }
    }
  }
}

// เรียกครั้งเดียวตอนเพิ่งสลับเข้าโหมดนี้ (ModeManager::update() คืน true)
inline void onEnter() {
  currentFrame = 0;
  lastFrameTime = millis();
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  setLeds(currentType);   // ติด LED ให้ตรงกับอารมณ์ปัจจุบันทันทีตอนเข้าโหมด
}

// เรียกตอนสลับออกจากโหมดนี้ - ดับ LED ทั้งหมดและกันเสียงค้างก่อนออกจากโหมด
inline void onExit() {
  noTone(BUZZER_PIN);
  digitalWrite(LED_YELLOW_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_RED_PIN, LOW);
}

// currentTemp: อุณหภูมิล่าสุดจาก DHT11 (อ่านใน main.cpp แล้วส่งเข้ามา)
inline void update(float currentTemp) {
  unsigned long now = millis();
  EmoteType newType = pickEmote(currentTemp, currentType);
  if (newType != currentType) {
    currentType = newType;
    currentFrame = 0;
    lastFrameTime = now;
    tone(BUZZER_PIN, EMOTE_BEEP_FREQ, EMOTE_BEEP_MS);   // บี๊บครั้งเดียวตอนอารมณ์เปลี่ยน (ไม่บล็อก loop)
    setLeds(currentType);                               // ดับ LED เดิม ติด LED ใหม่ตามอารมณ์
  }
  const Emote &e = EMOTES[currentType];
  if (now - lastFrameTime >= e.frames[currentFrame].ms) {
    lastFrameTime = now;
    currentFrame = (currentFrame + 1) % e.count;
  }
}

inline void draw(Adafruit_SSD1306 &display, float currentTemp, int xOffset) {
  const Emote &e = EMOTES[currentType];
  display.clearDisplay();
  drawBitmapScaled(display, xOffset + 16, 0, e.frames[currentFrame].bmp, 4);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print(currentTemp, 1);
  display.print(F(" C"));
  display.display();
}

}  // namespace EmoteMode