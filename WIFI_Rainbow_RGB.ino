#include <WiFi.h>          // 引入 WiFi 功能函式庫
#include <WebServer.h>     // 引入網頁伺服器功能函式庫
#include "Freenove_WS2812_Lib_for_ESP32.h" // 引入 LED 驅動函式庫 (針對 ESP32 優化)
#include "secrets.h"       // 引入私密資訊 (WiFi 名稱與密碼)

// --- 專案版本與標籤 ---
const char* TAG = "v16.2 [Pomo-ULTRA-LOW]";

// --- WiFi 設定 ---
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

// --- 硬體接腳與參數設定 ---
#define LED_PIN     10    
#define NUM_LEDS    12    
#define CHANNEL     0     
#define BRIGHTNESS  255   
#define BUTTON_PIN  9     

// --- 初始化物件 ---
Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(NUM_LEDS, LED_PIN, CHANNEL, TYPE_GRB);
WebServer server(80);

// --- 系統變數 ---
enum LEDMode { 
  MODE_OFF,     
  MODE_RAINBOW, 
  MODE_P12B, // 藍色 - 12m
  MODE_P24Y, // 黃色 - 24m
  MODE_P36R, // 紅色 - 36m
  MODE_P6G   // 綠色 - 6m
};

LEDMode currentMode = MODE_OFF; 
uint8_t rainbowStep = 0;        
unsigned long lastUpdate = 0;   
unsigned long lastTick = 0;     

unsigned long buttonDownTime = 0;
bool isLongPressHandled = false;

unsigned long pomoStartTime = 0;
unsigned long pomoDuration = 0; 
int pomoRemaining = 0;          
bool buttonState = true;        
bool lastButtonState = true;

// --- LED 控制函式 ---

void ledAllOff() {
  currentMode = MODE_OFF;
  for (int i = 0; i < NUM_LEDS; i++) strip.setLedColor(i, 0, 0, 0);
  strip.show();
  Serial.println("[LED] 全部熄滅");
}

void startPomodoro(LEDMode mode) {
  currentMode = mode;
  pomoStartTime = millis();
  if (mode == MODE_P12B) {
    pomoDuration = 12 * 60; 
    Serial.println("[番茄鐘] 藍色 (12min)");
  } else if (mode == MODE_P24Y) {
    pomoDuration = 24 * 60; 
    Serial.println("[番茄鐘] 黃色 (24min)");
  } else if (mode == MODE_P36R) {
    pomoDuration = 36 * 60; 
    Serial.println("[番茄鐘] 紅色 (36min)");
  } else if (mode == MODE_P6G) {
    pomoDuration = 6 * 60;  
    Serial.println("[番茄鐘] 綠色 (6min)");
  }
  pomoRemaining = pomoDuration;
}

void updatePomodoro() {
  unsigned long now = millis();
  
  if (now - lastTick >= 1000) { 
    lastTick = now;
    if (pomoRemaining > 0) pomoRemaining--;
    else {
      for(int j=0; j<3; j++) {
        for(int i=0; i<NUM_LEDS; i++) strip.setLedColor(i, 10, 10, 10); strip.show(); delay(200);
        for(int i=0; i<NUM_LEDS; i++) strip.setLedColor(i, 0, 0, 0); strip.show(); delay(200);
      }
      currentMode = MODE_OFF;
      return;
    }
  }

  int ledsToLight = (pomoRemaining * NUM_LEDS + pomoDuration - 1) / pomoDuration;

  // 極限低亮度：固定在 1 (WS2812 的發光物理底線)，取消呼吸以防閃爍感
  int bV = 1; 

  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < ledsToLight) {
      if (currentMode == MODE_P12B) strip.setLedColorData(i, 0, 0, bV); // 藍色
      else if (currentMode == MODE_P24Y) strip.setLedColorData(i, bV, bV, 0); // 黃色
      else if (currentMode == MODE_P36R) strip.setLedColorData(i, bV, 0, 0); // 紅色
      else if (currentMode == MODE_P6G) strip.setLedColorData(i, 0, bV, 0); // 綠色
    } else {
      strip.setLedColorData(i, 0, 0, 0);
    }
  }
  strip.show();
}

void updateRainbow() {
  for (int i = 0; i < NUM_LEDS; i++) {
    uint32_t color = strip.Wheel((i * 256 / NUM_LEDS + rainbowStep) & 255);
    byte r = (byte)((color >> 16) & 0xFF) / 50; // 極暗彩虹
    byte g = (byte)((color >> 8) & 0xFF) / 50;
    byte b = (byte)(color & 0xFF) / 50;
    strip.setLedColorData(i, r, g, b);
  }
  strip.show();
  rainbowStep += 1;
}

String getHTML() {
  String statusStr = "已熄滅";
  if (currentMode == MODE_RAINBOW) statusStr = "彩虹效果";
  else if (currentMode == MODE_P12B) statusStr = "藍色模式 (12m) 倒數中: " + String(pomoRemaining/60) + "m";
  else if (currentMode == MODE_P24Y) statusStr = "黃色模式 (24m) 倒數中: " + String(pomoRemaining/60) + "m";
  else if (currentMode == MODE_P36R) statusStr = "紅色模式 (36m) 倒數中: " + String(pomoRemaining/60) + "m";
  else if (currentMode == MODE_P6G) statusStr = "綠色模式 (6m) 倒數中: " + String(pomoRemaining/60) + "m";

  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Pomo-LED Ultra-Low</title><style>";
  html += "body { font-family: sans-serif; text-align: center; background: #000; color: #fff; padding-top: 50px; }";
  html += ".btn { display: block; width: 250px; margin: 15px auto; padding: 15px; color: #fff; text-decoration: none; border-radius: 50px; font-weight: bold; border: 1px solid #333; }";
  html += ".p1 { background: #0044ff; } .p2 { background: #aa8800; } .p3 { background: #aa0000; } .p4 { background: #008800; } .poff { background: #333; }";
  html += "</style></head><body>";
  html += "<h1>🍅 番茄鐘 LED (極暗版)</h1>";
  html += "<p>目前狀態: " + statusStr + "</p>";
  html += "<a href='/p12b' class='btn p1'>� 1. 藍色 (12m)</a>";
  html += "<a href='/p24y' class='btn p2'>🟡 2. 黃色 (24m)</a>";
  html += "<a href='/p36r' class='btn p3'>� 3. 紅色 (36m)</a>";
  html += "<a href='/p6g' class='btn p4'>🟢 4. 休息 (6m)</a>";
  html += "<a href='/off' class='btn poff'>全部熄滅</a>";
  html += "</body></html>";
  return html;
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  
  server.on("/", []() { server.send(200, "text/html", getHTML()); });
  server.on("/off", []() { ledAllOff(); server.sendHeader("Location", "/"); server.send(303); });
  server.on("/p12b", []() { startPomodoro(MODE_P12B); server.sendHeader("Location", "/"); server.send(303); });
  server.on("/p24y", []() { startPomodoro(MODE_P24Y); server.sendHeader("Location", "/"); server.send(303); });
  server.on("/p36r", []() { startPomodoro(MODE_P36R); server.sendHeader("Location", "/"); server.send(303); });
  server.on("/p6g", []() { startPomodoro(MODE_P6G); server.sendHeader("Location", "/"); server.send(303); });
  
  server.begin();
  strip.begin();
  strip.setBrightness(255);
  ledAllOff();
}

void loop() {
  server.handleClient();
  buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH) {
    buttonDownTime = millis();
    isLongPressHandled = false;
  }
  if (buttonState == HIGH && lastButtonState == LOW && !isLongPressHandled) {
    if (currentMode == MODE_OFF || currentMode == MODE_RAINBOW) startPomodoro(MODE_P12B);
    else if (currentMode == MODE_P12B) startPomodoro(MODE_P24Y);
    else if (currentMode == MODE_P24Y) startPomodoro(MODE_P36R);
    else if (currentMode == MODE_P36R) startPomodoro(MODE_P6G);
    else ledAllOff();
  }
  if (buttonState == LOW && !isLongPressHandled && (millis() - buttonDownTime > 1000)) {
    ledAllOff();
    isLongPressHandled = true;
  }
  lastButtonState = buttonState;

  if (currentMode >= MODE_P12B && currentMode <= MODE_P6G) updatePomodoro();
  else if (currentMode == MODE_RAINBOW && (millis() - lastUpdate > 30)) {
    lastUpdate = millis();
    updateRainbow();
  }
}