#include "Freenove_WS2812_Lib_for_ESP32.h"
#include <WiFi.h>

// --- 版本資訊 ---
const char* TAG = "v14.0 [Freenove RMT Logic]";

#define LED_PIN     10
#define NUM_LEDS    12
#define CHANNEL     0
#define BRIGHTNESS  10

// 使用 Freenove 函式庫，利用 ESP32 的 RMT 硬體驅動，避開計時干擾
Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(NUM_LEDS, LED_PIN, CHANNEL, TYPE_GRB);

bool rainbowEnabled = false;
uint8_t rainbowStep = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n*******************************************");
  Serial.println(TAG);
  Serial.println("策略：切換至 Freenove RMT 函式庫");
  Serial.println("*******************************************");

  // 雖然目前僅測試串列控制，但保留 WiFi 初始化以模擬真實應用場景
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); 
  
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  
  // 初始效果：先熄滅
  for (int i = 0; i < NUM_LEDS; i++) strip.setLedColor(i, 0, 0, 0);
  strip.show();

  Serial.println("\n請輸入指令：");
  Serial.println("[1] 全部點亮 (綠色)");
  Serial.println("[0] 全部熄滅");
  Serial.println("[2] 彩虹效果開/關");
}

void updateRainbow() {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setLedColorData(i, strip.Wheel((i * 256 / NUM_LEDS + rainbowStep) & 255));
  }
  strip.show();
  rainbowStep += 2;
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == '\n' || cmd == '\r') return;

    if (cmd == '1') {
      rainbowEnabled = false;
      Serial.println("[指令] 執行全部點亮...");
      for (int i = 0; i < NUM_LEDS; i++) strip.setLedColor(i, 0, 255, 100);
      strip.show();
    } 
    else if (cmd == '0') {
      rainbowEnabled = false;
      Serial.println("[指令] 執行全部熄滅...");
      for (int i = 0; i < NUM_LEDS; i++) strip.setLedColor(i, 0, 0, 0);
      strip.show();
    }
    else if (cmd == '2') {
      rainbowEnabled = !rainbowEnabled;
      Serial.print("[指令] 彩虹效果: ");
      Serial.println(rainbowEnabled ? "開啟" : "關閉");
      if (!rainbowEnabled) {
        for (int i = 0; i < NUM_LEDS; i++) strip.setLedColor(i, 0, 0, 0);
        strip.show();
      }
    }
  }

  if (rainbowEnabled) {
    updateRainbow();
    delay(10);
  }
}