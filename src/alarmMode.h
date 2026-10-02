#pragma once
#include <Adafruit_SSD1306.h>
#include "Button.h"

// โหมด 2: ตั้งเวลา (Alarm/Timer)
//
// ปุ่มหลักของระบบ (PIN 15 ผ่าน ModeManager) ใช้เปลี่ยนหน้าจอภายในโหมดนี้:
//   5 Min -> 10 Min -> 30 Min -> (กดอีกครั้ง = เกิน index สุดท้าย -> ขอสลับโหมด
//   ซึ่งตามลำดับปกติของ ModeManager คือไป Music ต่อ ตรงตาม "5->10->30->MusicMode->Loop")
//
// ปุ่ม OK (PIN 5) 2 หน้าที่:
//   - ตั้ง/ยกเลิก timer ได้เฉพาะตอนอยู่ในหน้าจอ Alarm (ต้องรู้ว่าเลือกกี่นาทีไว้)
//   - ปิดเสียงกริ่งที่ดังอยู่ได้จาก "ทุกโหมด" (ไม่งั้นถ้า alarm ดังตอนอยู่โหมดอื่นจะปิดไม่ได้)
//
// timer เดินและเช็คครบเวลาจาก main.cpp ที่เรียก update()/checkOkButton() "ทุกลูปเสมอ"
// ไม่ผูกกับโหมดปัจจุบัน ต่างจากเดิมที่เรียกเฉพาะตอนอยู่ในโหมด Alarm

#define OK_BUTTON_PIN    5
#define ALARM_BUZZER_PIN 25       // buzzer ตัวเดียวกับโหมดอื่น (Emote/Music ก็ใช้ขานี้)

enum AlarmScreen : uint8_t { SCREEN_5MIN = 0, SCREEN_10MIN, SCREEN_30MIN, SCREEN_COUNT };

static const unsigned long ALARM_MINUTES[SCREEN_COUNT] = { 5, 10, 30 };

// เสียงกริ่งเวลาครบ: บี๊บซ้ำเป็นจังหวะ (ไม่ใช่โทนเดียวยาว) เพื่อแย่งพิน buzzer
// กลับมาแจ้งเตือนซ้ำๆ ได้เรื่อยๆ แม้ระหว่างนั้นโหมดอื่น (เช่น Music) จะใช้ buzzer แทรกอยู่ก็ตาม
#define RING_BEEP_FREQ_HZ     2000
#define RING_BEEP_ON_MS       250
#define RING_BEEP_INTERVAL_MS 450

namespace AlarmMode {

inline Button okButton(OK_BUTTON_PIN);

inline uint8_t screenIndex = SCREEN_5MIN;

inline bool timerActive = false;
inline unsigned long timerStart = 0;
inline unsigned long timerDurationMs = 0;

inline bool alarmRinging = false;
inline unsigned long lastRingBeepTime = 0;

// เรียกครั้งเดียวตอนสลับ "เข้า" หน้าจอ Alarm (ModeManager ไปหยุดที่โหมดนี้)
// หมายเหตุ: ไม่แตะ timerActive/alarmRinging ตรงนี้ - ถ้ากำลังนับถอยหลัง/กริ่งอยู่
// ต้องคงสถานะไว้ต่อ ไม่ใช่โดนรีเซ็ตทุกครั้งที่กลับเข้ามาดูหน้าจอนี้
inline void onEnter() {
  Serial.println(F("Alarm mode"));
  pinMode(ALARM_BUZZER_PIN, OUTPUT);
  okButton.begin();
  screenIndex = SCREEN_5MIN;   // แค่หน้าจอ reset ทุกครั้งที่เข้า ไม่กระทบ timer/alarm
}

// เรียกตอนสลับ "ออก" จากหน้าจอ Alarm - ไม่ปิดเสียง/ไม่หยุด timer
// เพราะ alarm ต้องทำงานต่อเบื้องหลังได้แม้ไม่ได้อยู่หน้าจอนี้แล้ว
inline void onExit() {}

// เรียกจาก main.cpp ทุกครั้งที่กดปุ่มหลัก (PIN15) "ขณะอยู่ในโหมดนี้เท่านั้น"
// คืนค่า true เมื่อถึงตาที่ main.cpp ต้องสั่งเปลี่ยนโหมดจริง (เลื่อนเกินหน้าจอ 30 Min แล้ว)
inline bool onMainButtonPress() {
  uint8_t next = screenIndex + 1;
  if (next >= SCREEN_COUNT) {
    return true;              // เกินหน้าจอสุดท้าย -> ขอสลับโหมด
  }
  screenIndex = next;
  return false;
}

// เรียกจาก main.cpp "ทุกลูปเสมอ ไม่ว่าจะอยู่โหมดไหน"
// inAlarmScreen: true เฉพาะตอน modeManager.mode()==MODE_ALARM
//   - ปิดเสียงกริ่ง: ทำได้เสมอไม่ว่า inAlarmScreen จะเป็นอะไร
//   - ตั้ง/ยกเลิก timer ใหม่: ทำได้เฉพาะตอน inAlarmScreen==true เท่านั้น
inline void checkOkButton(bool inAlarmScreen) {
  if (!okButton.pressed()) return;

  if (alarmRinging) {
    alarmRinging = false;
    noTone(ALARM_BUZZER_PIN);
    return;
  }

  if (!inAlarmScreen) return;   // อยู่โหมดอื่น กด OK แล้วไม่มีผลถ้าไม่มีเสียงกริ่งให้ปิด

  if (timerActive) {
    timerActive = false;       // กด OK ซ้ำขณะนับถอยหลัง -> ยกเลิก timer
  } else {
    timerDurationMs = ALARM_MINUTES[screenIndex] * 60000UL;   // นาที -> มิลลิวินาที
    timerStart = millis();
    timerActive = true;
  }
}

// เรียกจาก main.cpp "ทุกลูปเสมอ ไม่ว่าจะอยู่โหมดไหน" เพื่อให้ timer เดินและกริ่งได้แม้ไม่ได้อยู่หน้า Alarm
inline void update(float /*currentTemp*/) {
  if (timerActive && millis() - timerStart >= timerDurationMs) {
    timerActive = false;
    alarmRinging = true;
    lastRingBeepTime = 0;   // บังคับให้บี๊บทันทีในลูปถัดไป
  }

  if (alarmRinging) {
    unsigned long now = millis();
    if (now - lastRingBeepTime >= RING_BEEP_INTERVAL_MS) {
      lastRingBeepTime = now;
      tone(ALARM_BUZZER_PIN, RING_BEEP_FREQ_HZ, RING_BEEP_ON_MS);
    }
  }
}

inline void draw(Adafruit_SSD1306 &display, int xOffset) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(xOffset + 2, 0);
  display.print(F("Alarm mode"));

  display.setCursor(xOffset + 2, 12);
  if (alarmRinging) {
    display.print(F("ALARM!"));
  } else {
    display.print(ALARM_MINUTES[screenIndex]);
    display.print(F(" Min"));
  }

  display.setCursor(xOffset + 2, 23);
  if (alarmRinging) {
    display.print(F("OK=stop"));
  } else if (timerActive) {
    unsigned long remainSec = (timerDurationMs - (millis() - timerStart)) / 1000UL;
    display.print(remainSec);
    display.print(F("s left"));
  } else {
    display.print(F("OK=set"));
  }

  display.display();
}

}  // namespace AlarmMode