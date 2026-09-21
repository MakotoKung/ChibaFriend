#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include "Emote.h"

#define SCREEN_I2C_ADDR 0x3C   // แก้ตามผล scanner
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define OLED_RST_PIN -1
#define EMOTE_SCALE 4          // 8x8 -> 32x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST_PIN);

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

float currentTemp = 25.0f;
EmoteType currentType = EMOTE_NORMAL;
uint8_t currentFrame = 0;
unsigned long lastFrameTime = 0;
unsigned long lastDHTReadTime = 0;
bool needRedraw = true;

// hysteresis 0.5 องศา กันภาพสลับไปมาตรงเส้นแบ่ง
EmoteType pickEmote(float t, EmoteType cur) {
  const float H = 0.5f;
  switch (cur) {
    case EMOTE_COLD:   return (t > 24.0f + H) ? EMOTE_NORMAL : EMOTE_COLD;
    case EMOTE_HOT:    return (t < 28.0f - H) ? EMOTE_NORMAL : EMOTE_HOT;
    default:
      if (t <= 27.0f - H) return EMOTE_COLD;
      if (t > 28.5f + H) return EMOTE_HOT;
      return EMOTE_NORMAL;
  }
}

void drawBitmapScaled(int x, int y, const uint8_t *bmp, int scale) {
  for (int r = 0; r < BMP_SIZE; r++) {
    uint8_t row = pgm_read_byte(bmp + r);
    for (int c = 0; c < BMP_SIZE; c++) {
      if (row & (0x80 >> c)) {
        display.fillRect(x + c * scale, y + r * scale, scale, scale, SSD1306_WHITE);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21,22);
  dht.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
}

void loop() {
  unsigned long now = millis();
  display.clearDisplay();
  
  if (now - lastDHTReadTime >= 2000UL) {
    lastDHTReadTime = now;
    float t = dht.readTemperature();
    if (!isnan(t)) currentTemp = t;
    Serial.printf("Temperature: %.1f C\n", currentTemp);
    needRedraw = true;
  }
  //Show Temperature
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 30);
  display.print(currentTemp, 1);
  display.print(F(" C"));

  EmoteType newType = pickEmote(currentTemp, currentType);
  if (newType != currentType) {
    currentType = newType;
    currentFrame = 0;          // รีเซ็ตเฟรมเมื่อเปลี่ยนอารมณ์
    lastFrameTime = now;
    needRedraw = true;
  }

  const Emote &e = EMOTES[currentType];
  if (now - lastFrameTime >= e.frames[currentFrame].ms) {
    lastFrameTime = now;
    currentFrame = (currentFrame + 1) % e.count;
    needRedraw = true;
  }

  if (needRedraw) {
    needRedraw = false;
    display.clearDisplay();
    drawBitmapScaled(16, 0, e.frames[currentFrame].bmp, EMOTE_SCALE);
    
    display.display();
  }
}