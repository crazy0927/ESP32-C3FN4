#include <WiFi.h>          // 引入 WiFi 功能函式庫
#include <WebServer.h>     // 引入網頁伺服器功能函式庫
#include "Freenove_WS2812_Lib_for_ESP32.h" // 引入 LED 驅動函式庫 (針對 ESP32 優化)
#include "secrets.h"       // 引入私密資訊 (WiFi 名稱與密碼)
#include <time.h>          // 引入時間函式庫

// --- 專案版本與標籤 ---
const char* TAG = "v17.0 [Button-Timer]";

// --- WiFi 設定 ---
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 28800;      
const int   daylightOffset_sec = 0;     

#define LED_PIN     10    
#define NUM_LEDS    12    
#define CHANNEL     0     
#define BRIGHTNESS  255   
#define BUTTON_PIN  9     

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(NUM_LEDS, LED_PIN, CHANNEL, TYPE_GRB);
WebServer server(80);

enum LEDMode { 
  MODE_OFF,     
  MODE_RAINBOW, 
  MODE_CLOCK,
  MODE_TIMER_SET, // 紫色設定模式
  MODE_TIMER_RUN, // 執行自定義倒數
  MODE_P12B, MODE_P24Y, MODE_P36R, MODE_P6G 
};

LEDMode currentMode = MODE_OFF; 
uint8_t rainbowStep = 0;        
unsigned long lastUpdate = 0;   
unsigned long lastTick = 0;     

unsigned long buttonDownTime = 0;
bool isLongPressHandled = false;

// 倒數計時變數
unsigned long pomoDuration = 0; 
int pomoRemaining = 0;          

// 自定義設定變數
int customSetLeds = 0;          // 設定時點亮的 LED 數量
unsigned long lastButtonAction = 0; 

bool buttonState = true;        
bool lastButtonState = true;

void ledAllOff() {
  currentMode = MODE_OFF;
  for (int i = 0; i < NUM_LEDS; i++) strip.setLedColor(i, 0, 0, 0);
  strip.show();
  Serial.println("[LED] 全部熄滅");
}

void startTimer(int minutes, LEDMode mode) {
  currentMode = mode;
  pomoDuration = minutes * 60;
  pomoRemaining = pomoDuration;
  lastTick = millis();
  Serial.printf("[計時器] 開始倒數: %d 分鐘\n", minutes);
}

void updatePomodoro() {
  unsigned long now = millis();
  if (now - lastTick >= 1000) { 
    lastTick = now;
    if (pomoRemaining > 0) pomoRemaining--;
    else {
      // 結束閃爍提醒
      for(int j=0; j<3; j++) {
        for(int i=0; i<NUM_LEDS; i++) strip.setLedColor(i, 10, 10, 10); strip.show(); delay(200);
        for(int i=0; i<NUM_LEDS; i++) strip.setLedColor(i, 0, 0, 0); strip.show(); delay(200);
      }
      currentMode = MODE_CLOCK; // 結束後回到時鐘
      return;
    }
  }

  int ledsToLight = (pomoRemaining * NUM_LEDS + pomoDuration - 1) / pomoDuration;
  int bV = 1; 

  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < ledsToLight) {
      if (currentMode == MODE_P12B) strip.setLedColorData(i, 0, 0, bV);
      else if (currentMode == MODE_P24Y) strip.setLedColorData(i, bV, bV, 0);
      else if (currentMode == MODE_P36R) strip.setLedColorData(i, bV, 0, 0);
      else if (currentMode == MODE_P6G) strip.setLedColorData(i, 0, bV, 0);
      else if (currentMode == MODE_TIMER_RUN) strip.setLedColorData(i, bV, 0, bV); // 自定義顯示紫色
    } else {
      strip.setLedColorData(i, 0, 0, 0);
    }
  }
  strip.show();
}

void updateClock() {
  struct tm ti;
  if (!getLocalTime(&ti)) {
    for (int i = 0; i < NUM_LEDS; i++) strip.setLedColorData(i, 1, 1, 0);
    strip.show();
    return;
  }
  int h = ti.tm_hour % 12, m = ti.tm_min / 5, s = ti.tm_sec / 5;
  for (int i = 0; i < NUM_LEDS; i++) {
    int r=0, g=0, b=0;
    if (i == h) { r+=1; g+=1; b+=1; }
    if (i == m) { g+=1; b+=1; }
    if (i == s) { r+=1; }
    strip.setLedColorData(i, (r>2?2:r), (g>2?2:g), (b>2?2:b));
  }
  strip.show();
}

void updateTimerSetDisplay() {
  // 紫色設定介面，每顆燈代表 5 分鐘
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < customSetLeds) strip.setLedColorData(i, 2, 0, 2); // 亮紫色
    else if (i == customSetLeds) {
       // 正在設定的那顆燈微弱呼吸或閃爍
       int pulse = (millis() % 1000 < 500) ? 1 : 0;
       strip.setLedColorData(i, pulse, 0, pulse);
    } else strip.setLedColorData(i, 0, 0, 0);
  }
  strip.show();
}

String getHTML() {
  String statusStr = "模式中";
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<style>body{font-family:sans-serif;text-align:center;background:#000;color:#fff;padding-top:50px;}";
  html += ".btn{display:block;width:250px;margin:15px auto;padding:15px;color:#fff;text-decoration:none;border-radius:50px;font-weight:bold;border:1px solid #333;}";
  html += ".p1{background:#0044ff;}.p2 {background:#aa8800;}.p3 {background:#aa0000;}.p4 {background:#008800;}.pclock{background:#444;}.poff{background:#222;}</style></head><body>";
  html += "<h1>🕒 時鐘 & 🍅 番茄鐘</h1>";
  if (currentMode == MODE_CLOCK) {
    struct tm ti; if (getLocalTime(&ti)) {
      char timeStr[20]; sprintf(timeStr, "%02d:%02d:%02d", ti.tm_hour, ti.tm_min, ti.tm_sec);
      html += "<h2 style='font-size:3em;margin:10px;color:#0ff;'>" + String(timeStr) + "</h2>";
    }
  }
  html += "<p>透過按鈕可進入『自定義計時』模式：<br>紫色燈閃爍時，點擊增加 5 分鐘，靜置 2 秒啟動。</p>";
  html += "<a href='/clock' class='btn pclock'>🕒 時鐘模式</a>";
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
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  server.on("/", []() { server.send(200, "text/html", getHTML()); });
  server.on("/off", []() { ledAllOff(); server.sendHeader("Location", "/"); server.send(303); });
  server.on("/clock", []() { currentMode = MODE_CLOCK; server.sendHeader("Location", "/"); server.send(303); });
  
  server.begin();
  strip.begin();
  currentMode = MODE_CLOCK;
  ledAllOff();
  currentMode = MODE_CLOCK;
}

void loop() {
  server.handleClient();
  buttonState = digitalRead(BUTTON_PIN);
  
  // 按鍵按下瞬間
  if (buttonState == LOW && lastButtonState == HIGH) {
    buttonDownTime = millis();
    isLongPressHandled = false;
  }
  
  // 按鍵放開瞬間 (短按邏輯)
  if (buttonState == HIGH && lastButtonState == LOW && !isLongPressHandled) {
    if (currentMode == MODE_OFF) {
      currentMode = MODE_CLOCK;
    } else if (currentMode == MODE_CLOCK) {
      currentMode = MODE_TIMER_SET;
      customSetLeds = 1; // 預設 5 分鐘
      lastButtonAction = millis();
    } else if (currentMode == MODE_TIMER_SET) {
      customSetLeds = (customSetLeds % 12) + 1; // 增加 5 分鐘，最多 60 分
      lastButtonAction = millis();
    } else if (currentMode == MODE_TIMER_RUN) {
      startTimer(12, MODE_P12B); // 切換到固定的番茄鐘模式
    } else if (currentMode == MODE_P12B) startTimer(24, MODE_P24Y);
      else if (currentMode == MODE_P24Y) startTimer(36, MODE_P36R);
      else if (currentMode == MODE_P36R) startTimer(6, MODE_P6G);
      else ledAllOff();
  }

  // 長按 1 秒熄滅
  if (buttonState == LOW && !isLongPressHandled && (millis() - buttonDownTime > 1000)) {
    ledAllOff();
    isLongPressHandled = true;
  }
  lastButtonState = buttonState;

  // 自定義計時器設定超時判斷 (停按 2 秒啟動)
  if (currentMode == MODE_TIMER_SET && (millis() - lastButtonAction > 2000)) {
    startTimer(customSetLeds * 5, MODE_TIMER_RUN);
  }

  // 模式運行
  if (currentMode == MODE_CLOCK) updateClock();
  else if (currentMode == MODE_TIMER_SET) updateTimerSetDisplay();
  else if (currentMode >= MODE_TIMER_RUN && currentMode <= MODE_P6G) updatePomodoro();
}