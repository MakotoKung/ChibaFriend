#pragma once
#include <Arduino.h>

// โหมดทั้งหมดของอุปกรณ์
enum AppMode : uint8_t {
  MODE_EMOTE = 1,   // default: หน้าจอ emote ตามอุณหภูมิ
  MODE_ALARM = 2,   // พิมพ์ "Alarm mode"
  MODE_MUSIC = 3,   // เมนูเลือกเพลง + เล่นเพลง
};

class ModeManager {
public:
  // buttonPin: ขา GPIO ของปุ่ม (active-low -> ใช้ INPUT_PULLUP)
  explicit ModeManager(uint8_t buttonPin) : _pin(buttonPin) {}

  void begin() {
    pinMode(_pin, INPUT_PULLUP);
    _lastRaw = digitalRead(_pin);
    _stableState = _lastRaw;
  }

  // เรียกทุกรอบของ loop() - คืน true "ครั้งเดียว" ต่อการกดปุ่มหนึ่งครั้ง (debounced edge)
  // ฟังก์ชันนี้แค่รายงานว่ามีการกด ไม่ได้ตัดสินใจเปลี่ยนโหมดเอง
  // (เดิม MusicMode ต้องดักปุ่มไปทำเมนูของตัวเองก่อน จึงแยกออกจาก advanceMode())
  bool buttonPressed() {
    bool raw = digitalRead(_pin);
    unsigned long now = millis();

    if (raw != _lastRaw) {
      _lastRaw = raw;
      _lastChangeTime = now;
    }

    bool pressed = false;
    if ((now - _lastChangeTime) > DEBOUNCE_MS && raw != _stableState) {
      _stableState = raw;
      if (_stableState == LOW) pressed = true;   // ขอบขาลง = กดจริง
    }
    return pressed;
  }

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
  static constexpr unsigned long DEBOUNCE_MS = 30;

  uint8_t _pin;
  bool _lastRaw = HIGH;
  bool _stableState = HIGH;
  unsigned long _lastChangeTime = 0;
  AppMode _mode = MODE_EMOTE;   // default
};