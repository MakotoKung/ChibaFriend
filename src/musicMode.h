#pragma once
#include <Adafruit_SSD1306.h>

// โหมด 3: เล่นเพลงแบบ non-blocking
//
// ปุ่มเดียวใช้เลื่อนไปเพลงถัดไป แล้ว "เล่นทันที" ไม่ต้องมีหน้า Play แยก:
//   Song1 (เล่นอัตโนมัติตอนเข้าโหมด) -> กด -> Song2 (เล่นทันที) -> กด -> Song3 (เล่นทันที)
//   -> กดอีกครั้ง (เกิน index สุดท้าย) -> ขอออกจากโหมด Music
// main.cpp เป็นคนสั่ง ModeManager::advanceMode() จริงๆ ฟังก์ชัน onButtonPress()
// แค่คืนค่า true บอกว่า "ถึงตาสลับโหมดแล้ว"

#define BUZZER_PIN 25

// ---------------- ข้อมูลเพลง ----------------
// !! ลิขสิทธิ์ !!
// "Haggstrom" และ "Subwoofer Lullaby" เป็นเพลงของ C418 (Minecraft) มีลิขสิทธิ์
// จึงไม่ใส่โน้ตทีละตัว (note-for-note) ให้ในนี้ - เว้น array ไว้เป็น placeholder
// ให้คุณใส่โน้ต/จังหวะของตัวเองตรงนี้ (หาได้จากเว็บแปลง MIDI -> Arduino tone()
// ของเพลงที่คุณมีสิทธิ์ใช้ หรือแต่งทำนองสั้นๆ ของตัวเอง)

// เพลง 1: Haggstrom - ใส่โน้ตจริงแทนที่ 0 ด้านล่าง (0 = พัก/silence)
#define NOTE_E5  659
#define NOTE_C5  523
#define NOTE_G5  784
#define NOTE_G4  392
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_C3  131
#define NOTE_F4  349
#define NOTE_F5  698
#define NOTE_A5  880
#define NOTE_GB4 242

#define REST     0
// เพลง 1: Haggstrom - ใส่โน้ตจริงแทนที่ 0 ด้านล่าง (0 = พัก/silence)
static const int NOTE_SONG1[] = { 
    NOTE_D4, NOTE_D4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_A4, REST,
  NOTE_A4, NOTE_A4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_D5, NOTE_D5, REST,
  NOTE_D5, NOTE_C5, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_F4, NOTE_E4, REST,
  NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, REST, NOTE_A4, REST
 };   // TODO: ใส่โน้ตเพลง Haggstrom
static const int DUR_SONG1[]  = {  
  8, 8, 8, 8, 4, 8, 4, 8,
  8, 8, 8, 8, 4, 8, 4, 8,
  8, 8, 8, 8, 4, 8, 4, 8,
  8, 8, 8, 8, 4, 8, 4, 8 };   // TODO: ใส่จังหวะคู่กับโน้ตด้านบน (ความยาวต้องเท่ากับ NOTE_SONG1)

// เพลง 3: Subwoofer Lullaby - ใส่โน้ตจริงแทนที่ 0 ด้านล่าง (0 = พัก/silence)
static const int NOTE_SONG3[] = { 
    NOTE_B4, NOTE_B4, NOTE_B4, NOTE_GB4, NOTE_E5};   // TODO: ใส่โน้ตเพลง Subwoofer Lullaby
static const int DUR_SONG3[]  = { 
    8, 8, 8, 8, 8 };   // TODO: ใส่จังหวะคู่กับโน้ตด้านบน (ความยาวต้องเท่ากับ NOTE_SONG3)

// เพลง 2: ทำนองเดิมที่ใช้อยู่ (เก็บไว้ตามเดิม)

static const int NOTE_SONG2[] = {
  NOTE_E5, NOTE_E5, 0,
  NOTE_E5, 0, NOTE_C5, NOTE_E5,
  0, NOTE_G5, 0, 0, 0,
  NOTE_G4, 0, 0, 0
};
static const int DUR_SONG2[] = {
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12
};


struct Song {
  const int *notes;
  const int *durations;
  int length;
  const char *name;       // ใช้แสดงบนจอ (สั้น ๆ ให้พอดีกับจอ 64px)
};

static const Song SONGS[] = {
  { NOTE_SONG1, DUR_SONG1, sizeof(NOTE_SONG1) / sizeof(int), "Haggstrom" },
  { NOTE_SONG2, DUR_SONG2, sizeof(NOTE_SONG2) / sizeof(int), "Song 2" },
  { NOTE_SONG3, DUR_SONG3, sizeof(NOTE_SONG3) / sizeof(int), "Subwoofer" },
};
static const uint8_t SONG_COUNT = sizeof(SONGS) / sizeof(Song);

namespace MusicMode {

inline uint8_t currentSong = 0;
inline bool playing = false;

inline int currentNote = 0;
inline unsigned long noteStartTime = 0;
inline unsigned long currentGapMs = 0;

inline void stopPlayback() {
  noTone(BUZZER_PIN);
  playing = false;
}

inline void startNote(int i) {
  const Song &s = SONGS[currentSong];
  currentNote = i;
  int noteDuration = 1000 / s.durations[currentNote];
  currentGapMs = (unsigned long)(noteDuration * 1.30f);

  if (s.notes[currentNote] == 0) {
    noTone(BUZZER_PIN);
  } else {
    tone(BUZZER_PIN, s.notes[currentNote], noteDuration);
  }
  noteStartTime = millis();
}

inline void startSong(uint8_t songIdx) {
  currentSong = songIdx;
  playing = true;
  startNote(0);
}

inline void onEnter() {
  Serial.println(F("Music"));
  pinMode(BUZZER_PIN, OUTPUT);
  startSong(0);   // เข้าโหมดใหม่ทุกครั้ง -> เริ่มเล่นเพลงที่ 1 ทันที
}

inline void onExit() {
  stopPlayback();
}

// เรียกจาก main.cpp ทุกครั้งที่มีการกดปุ่ม "ขณะอยู่ในโหมด Music"
// คืนค่า true เมื่อถึงตาที่ main.cpp ต้องสั่งเปลี่ยนโหมดจริง (กดเกินเพลงสุดท้ายแล้ว)
inline bool onButtonPress() {
  uint8_t next = currentSong + 1;
  if (next >= SONG_COUNT) {
    return true;              // หมด index เพลงแล้ว -> ขอสลับโหมด
  }
  startSong(next);             // ยังมีเพลงถัดไป -> เล่นทันที
  return false;
}

inline void update(float /*currentTemp*/) {
  if (!playing) return;
  const Song &s = SONGS[currentSong];
  if (millis() - noteStartTime < currentGapMs) return;

  int next = currentNote + 1;
  if (next >= s.length) next = 0;   // เล่นจบ -> วนเพลงเดิมซ้ำ
  startNote(next);
}

inline void draw(Adafruit_SSD1306 &display, int xOffset) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // "Music mode" กลางบนสุด
  display.setTextSize(1);
  display.setCursor(xOffset + 2, 0);
  display.print(F("Music mode"));

  // ชื่อเพลงที่กำลังเล่น
  display.setCursor(xOffset + 2, 12);
  display.print(F("> "));
  display.print(SONGS[currentSong].name);

  // สถานะ
  display.setCursor(xOffset + 2, 23);
  display.print(playing ? F("Playing") : F("Stopped"));

  display.display();
}

}  // namespace MusicMode