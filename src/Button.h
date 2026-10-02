#pragma once
#include <Arduino.h>

// ปุ่มแบบ active-low พร้อม debounce แบบ non-blocking (ไม่ใช้ delay())
// เรียก pressed() ทุกรอบของ loop() - คืน true "ครั้งเดียว" ต่อการกดหนึ่งครั้ง (ขอบขาลง)
// แยกออกมาเป็นคลาสกลาง เพราะทั้ง ModeManager (ปุ่มหลัก) และ AlarmMode (ปุ่ม OK)
// ต้องใช้ logic debounce แบบเดียวกัน
class Button {
public:
  explicit Button(uint8_t pin) : _pin(pin) {}

  void begin() {
    pinMode(_pin, INPUT_PULLUP);
    _lastRaw = digitalRead(_pin);
    _stableState = _lastRaw;
  }

  bool pressed() {
    bool raw = digitalRead(_pin);
    unsigned long now = millis();

    if (raw != _lastRaw) {
      _lastRaw = raw;
      _lastChangeTime = now;
    }

    bool result = false;
    if ((now - _lastChangeTime) > DEBOUNCE_MS && raw != _stableState) {
      _stableState = raw;
      if (_stableState == LOW) result = true;   // ขอบขาลง = กดจริง
    }
    return result;
  }

private:
  static constexpr unsigned long DEBOUNCE_MS = 30;

  uint8_t _pin;
  bool _lastRaw = HIGH;
  bool _stableState = HIGH;
  unsigned long _lastChangeTime = 0;
};