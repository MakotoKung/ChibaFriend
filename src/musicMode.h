#pragma once
#include <Adafruit_SSD1306.h>

// โหมด 3: ยังไม่ทำ logic จริง แค่แสดงข้อความไว้ก่อน
// เพิ่มฟังก์ชันการทำงานจริง (เช่น เล่นเสียง, VU meter) ได้ในไฟล์นี้ทีหลัง
namespace MusicMode {

inline void onEnter() {
  Serial.println(F("Music"));
}

inline void update(float /*currentTemp*/) {
  // TODO: ใส่ logic ของโหมด Music
}

inline void draw(Adafruit_SSD1306 &display, int xOffset) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(xOffset + 4, 12);
  display.print(F("Music"));
  display.display();
}

}  // namespace MusicMode