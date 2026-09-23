#pragma once
#include <Adafruit_SSD1306.h>
#include "Emote.h"

// ระยะห่างของ bitmap จากขอบซ้ายของพื้นที่แสดงผลจริง (หลังชดเชย offset แล้ว)
namespace EmoteMode {

inline EmoteType currentType = EMOTE_NORMAL;
inline uint8_t currentFrame = 0;
inline unsigned long lastFrameTime = 0;

// hysteresis 0.5 องศา กันภาพสลับไปมาตรงเส้นแบ่ง
inline EmoteType pickEmote(float t, EmoteType cur) {
  const float H = 0.5f;
  switch (cur) {
    case EMOTE_COLD: return (t >= 27.0f + H) ? EMOTE_NORMAL : EMOTE_COLD;
    case EMOTE_HOT:  return (t < 28.5f - H) ? EMOTE_NORMAL : EMOTE_HOT;
    default:
      if (t <= 27.0f - H) return EMOTE_COLD;
      if (t >  28.5f + H) return EMOTE_HOT;
      return EMOTE_NORMAL;
  }
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
}
inline void onExit() {}

// currentTemp: อุณหภูมิล่าสุดจาก DHT11 (อ่านใน main.cpp แล้วส่งเข้ามา)
inline void update(float currentTemp) {
  unsigned long now = millis();
  EmoteType newType = pickEmote(currentTemp, currentType);
  if (newType != currentType) {
    currentType = newType;
    currentFrame = 0;
    lastFrameTime = now;
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