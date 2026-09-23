#pragma once
#include <Arduino.h>

// โหมดทั้งหมดของอุปกรณ์ กด BUTTON_PIN เพื่อวนลำดับ 1 -> 2 -> 3 -> 1
enum AppMode : uint8_t {
  MODE_EMOTE = 1,   // default: หน้าจอ emote ตามอุณหภูมิ
  MODE_ALARM = 2,   // พิมพ์ "Alarm mode"
  MODE_MUSIC = 3,   // พิมพ์ "Music"
};

class ModeManager {
public:
  // buttonPin: ขา GPIO ของปุ่ม (active-low -> ใช้ INPUT_PULLUP)
  explicit ModeManager(uint8_t buttonPin) : _pin(buttonPin) {}

  void begin() {
    pinMode(_pin, INPUT_PULLUP);
    _lastRaw = digitalRead(_pin);
  }

  // เรียกทุกรอบของ loop() ก่อนใช้งาน mode()
  // คืนค่า true ถ้าโหมดเพิ่งเปลี่ยนในรอบนี้ (ให้แต่ละโหมดใช้รีเซ็ตสถานะของตัวเอง)
  bool update() {
    bool raw = digitalRead(_pin);   // active-low: LOW = กด
    unsigned long now = millis();

    if (raw != _lastRaw) {
      _lastRaw = raw;
      _lastChangeTime = now;
    }

    bool changed = false;
    if ((now - _lastChangeTime) > DEBOUNCE_MS && raw != _stableState) {
      _stableState = raw;
      if (_stableState == LOW) {          // ขอบขาลง = กดปุ่มจริง
        _mode = nextMode(_mode);
        changed = true;
      }
    }
    return changed;
  }

  AppMode mode() const { return _mode; }

private:
  static AppMode nextMode(AppMode m) {
    switch (m) {
      case MODE_EMOTE: return MODE_ALARM;
      case MODE_ALARM: return MODE_MUSIC;
      default:         return MODE_EMOTE;
    }
  }

  static constexpr unsigned long DEBOUNCE_MS = 30;

  uint8_t _pin;
  bool _lastRaw = HIGH;
  bool _stableState = HIGH;
  unsigned long _lastChangeTime = 0;
  AppMode _mode = MODE_EMOTE;   // default
};