#ifndef EMOTE_H
#define EMOTE_H

#include <Arduino.h>

#define BMP_SIZE 8

// ---- Normal ----
static const uint8_t PROGMEM n_open[] = {
  0b00111100, 0b01000010, 0b10100101, 0b10000001,
  0b10111101, 0b10000001, 0b01000010, 0b00111100
};
static const uint8_t PROGMEM n_blink[] = {
  0b00111100, 0b01000010, 0b10000001, 0b10000001,
  0b10111101, 0b10000001, 0b01000010, 0b00111100
};

// ---- Cold (ปากคว่ำ + สั่น) ----
static const uint8_t PROGMEM c_a[] = {
  0b00111100, 0b01000010, 0b10100101, 0b10000001,
  0b10011001, 0b10100101, 0b01000010, 0b00111100
};
static const uint8_t PROGMEM c_b[] = {
  0b00111100, 0b01000010, 0b10100101, 0b10000001,
  0b10111101, 0b10100101, 0b01000010, 0b00111100
};

// ---- Hot (ปากอ้า หอบ) ----
static const uint8_t PROGMEM h_a[] = {
  0b00111100, 0b01000010, 0b10100101, 0b10000001,
  0b10011001, 0b10011001, 0b01000010, 0b00111100
};
static const uint8_t PROGMEM h_b[] = {
  0b00111100, 0b01000010, 0b10100101, 0b10000001,
  0b10111101, 0b10011001, 0b01000010, 0b00111100
};

struct Frame {
  const uint8_t *bmp;
  uint16_t ms;  // เวลาที่ค้างเฟรมนี้
};

struct Emote {
  const Frame *frames;
  uint8_t count;
};

static const Frame FRAMES_NORMAL[] = { {n_open, 1800}, {n_blink, 120} };
static const Frame FRAMES_COLD[]   = { {c_a, 150}, {c_b, 150} };
static const Frame FRAMES_HOT[]    = { {h_a, 350}, {h_b, 350} };

enum EmoteType { EMOTE_COLD, EMOTE_NORMAL, EMOTE_HOT };

static const Emote EMOTES[] = {
  { FRAMES_COLD, 2 },
  { FRAMES_NORMAL, 2 },
  { FRAMES_HOT, 2 },
};

#endif