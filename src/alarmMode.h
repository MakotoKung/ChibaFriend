#pragma once
#include <Adafruit_SSD1306.h>

// โหมด 2: ยังไม่ทำ logic จริง แค่แสดงข้อความไว้ก่อน
// เพิ่มฟังก์ชันการทำงานจริง (เช่น ตั้งเวลา, สั่น, บัซเซอร์) ได้ในไฟล์นี้ทีหลัง
namespace AlarmMode {

inline void onEnter() {
  Serial.println(F("Alarm mode"));
}
inline void onExit() {}

inline void update(float /*currentTemp*/) {
  // TODO: ใส่ logic ของโหมด Alarm
}

inline void draw(Adafruit_SSD1306 &display, int xOffset) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(xOffset + 4, 12);
  display.print(F("Alarm mode"));
  display.display();
}

}  // namespace AlarmMode