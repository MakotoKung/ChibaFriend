#pragma once
#include <Arduino.h>
#include "Button.h"

// โหมดทั้งหมดของอุปกรณ์
enum AppMode : uint8_t {
  MODE_EMOTE = 1,   // default: หน้าจอ emote ตามอุณหภูมิ
  MODE_ALARM = 2,   // ตั้งเวลา (5/10/30 นาที)
  MODE_MUSIC = 3,   // เล่นเพลง
};

class ModeManager {
public:
  // buttonPin: ขา GPIO ของปุ่มหลัก (active-low)
  explicit ModeManager(uint8_t buttonPin) : _button(buttonPin) {}

  void begin() { _button.begin(); }

  // เรียกทุกรอบของ loop() - คืน true "ครั้งเดียว" ต่อการกดปุ่มหนึ่งครั้ง (debounced edge)
  // ฟังก์ชันนี้แค่รายงานว่ามีการกด ไม่ได้ตัดสินใจเปลี่ยนโหมดเอง
  // (Alarm/Music ต้องดักปุ่มไปทำเมนู/หน้าจอของตัวเองก่อน จึงแยกออกจาก advanceMode())
  bool buttonPressed() { return _button.pressed(); }

  // สลับไปโหมดถัดไป (1 -> 2 -> 3 -> 1) - เรียกเมื่อ "ยืนยัน" จะเปลี่ยนโหมดแล้วเท่านั้น
  void advanceMode() {
    switch (_mode) {
      case MODE_EMOTE: _mode = MODE_ALARM; break;
      case MODE_ALARM: _mode = MODE_MUSIC; break;
      default:         _mode = MODE_EMOTE; break;
    }
  }

  AppMode mode() const { return _mode; }

private:
  Button _button;
  AppMode _mode = MODE_EMOTE;   // default
};