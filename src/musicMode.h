#pragma once
#include <Adafruit_SSD1306.h>

// โหมด 3: เล่นเมโลดี้ผ่าน buzzer แบบ non-blocking
// เล่นทีละโน้ตต่อการเรียก update() หนึ่งครั้ง ไม่ใช้ delay()
// เพื่อให้ loop() ยังวิ่งเช็คปุ่มได้ตลอด แม้เพลงกำลังเล่นอยู่

#define BUZZER_PIN 25
#define NOTE_E5  659
#define NOTE_C5  523
#define NOTE_G5  784
#define NOTE_G4  392

// Melody array เก็บโน้ตของเพลง (0 = พัก/silence)
static const int melody[] = {
  NOTE_E5, NOTE_E5, 0,
  NOTE_E5, 0, NOTE_C5, NOTE_E5,
  0, NOTE_G5, 0, 0, 0,
  NOTE_G4, 0, 0, 0
};

// ความยาวของโน้ต (ยิ่งค่าน้อย เสียงยิ่งยาว เช่น 12 = เล่นเร็ว) คู่ index กับ melody[]
static const int noteDurations[] = {
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12
};

static const int TOTAL_NOTES = sizeof(melody) / sizeof(melody[0]);

namespace MusicMode {

inline int currentNote = 0;
inline unsigned long noteStartTime = 0;
inline unsigned long currentGapMs = 0;   // เวลาที่ต้องรอ (โน้ต + ช่องว่าง 30%) ก่อนขยับโน้ตถัดไป

// เริ่มเล่นโน้ตที่ index i (ตั้งแต่ i=0 ถึง TOTAL_NOTES-1); วนกลับ 0 เมื่อครบเพลง
inline void startNote(int i) {
  currentNote = i;
  int noteDuration = 1000 / noteDurations[currentNote];
  currentGapMs = (unsigned long)(noteDuration * 1.30f);

  if (melody[currentNote] == 0) {
    noTone(BUZZER_PIN);            // โน้ตนี้เป็นช่องว่าง (เงียบ)
  } else {
    tone(BUZZER_PIN, melody[currentNote], noteDuration);
  }
  noteStartTime = millis();
}

inline void onEnter() {
  Serial.println(F("Music"));
  pinMode(BUZZER_PIN, OUTPUT);
  startNote(0);                    // เริ่มเพลงใหม่ทุกครั้งที่เข้าโหมดนี้
}

// เรียกตอนสลับออกจากโหมดนี้ ป้องกันเสียงค้างถ้ากดสลับโหมดขณะโน้ตกำลังดัง
inline void onExit() {
  noTone(BUZZER_PIN);
}

inline void update(float /*currentTemp*/) {
  if (millis() - noteStartTime < currentGapMs) return;   // โน้ตปัจจุบันยังไม่ครบเวลา -> รอ

  int next = currentNote + 1;
  if (next >= TOTAL_NOTES) next = 0;   // เล่นจบ -> วนเพลงใหม่
  startNote(next);
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