#include <WiFi.h>          // 引入 WiFi 功能函式庫
#include <WebServer.h>     // 引入網頁伺服器功能函式庫
#include "Freenove_WS2812_Lib_for_ESP32.h" // 引入 LED 驅動函式庫 (針對 ESP32 優化)
#include "secrets.h"       // 引入私密資訊 (WiFi 名稱與密碼)
#include <time.h>          // 引入時間函式庫

// --- 專案版本與標籤 ---
const char* TAG = "v18.1 [Serial-Debug]";

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
  MODE_P12B, MODE_P24Y, MODE_P36R, MODE_P6G, 
  MODE_TIMER_SET, 
  MODE_TIMER_RUN  
};

LEDMode currentMode = MODE_OFF; 
uint8_t rainbowStep = 0;        
unsigned long lastUpdate = 0;   
unsigned long lastTick = 0;     

unsigned long buttonDownTime = 0;
bool isLongPressHandled = false;

unsigned long pomoDuration = 0; 
int pomoRemaining = 0;          

int customSetLeds = 0;          
unsigned long lastButtonAction = 0; 

bool buttonState = true;        
bool lastButtonState = true;

// --- 輔助函式：取得模式名稱 ---
String getModeName(LEDMode mode) {
  switch(mode) {
    case MODE_OFF: return "熄滅";
    case MODE_CLOCK: return "網路時鐘";
    case MODE_P12B: return "12分番茄鐘";
    case MODE_P24Y: return "24分番茄鐘";
    case MODE_P36R: return "36分番茄鐘";
    case MODE_P6G: return "6分休息";
    case MODE_TIMER_SET: return "設定計時器中";
    case MODE_TIMER_RUN: return "自定義倒數";
    default: return "未知";
  }
}

void ledAllOff() {
  currentMode = MODE_OFF;
  for (int i = 0; i < NUM_LEDS; i++) strip.setLedColor(i, 0, 0, 0);
  strip.show();
  Serial.println(">>> [系統] LED 全部熄滅");
}

void startTimer(int minutes, LEDMode mode) {
  currentMode = mode;
  pomoDuration = (unsigned long)minutes * 60;
  pomoRemaining = pomoDuration;
  lastTick = millis();
  Serial.print(">>> [模式切換] ");
  Serial.print(getModeName(mode));
  Serial.printf(" | 設定時間: %d 分鐘\n", minutes);
}

void updatePomodoro() {
  unsigned long now = millis();
  if (now - lastTick >= 1000) { 
    lastTick = now;
    if (pomoRemaining > 0) {
      pomoRemaining--;
      // 每 30 秒在序號埠輸出一報時，避免洗掉視窗
      if (pomoRemaining % 30 == 0) {
        Serial.printf(">>> [倒數中] 剩餘: %02d:%02d\n", pomoRemaining/60, pomoRemaining%60);
      }
    } else {
      Serial.println(">>> [系統] 時間到！觸發閃爍提醒");
      for(int j=0; j<3; j++) {
        for(int i=0; i<NUM_LEDS; i++) strip.setLedColor(i, 10, 10, 10); strip.show(); delay(200);
        for(int i=0; i<NUM_LEDS; i++) strip.setLedColor(i, 0, 0, 0); strip.show(); delay(200);
      }
      currentMode = MODE_CLOCK; 
      Serial.println(">>> [系統] 回到時鐘模式");
      return;
    }
  }

  int ledsToLight = (pomoRemaining * NUM_LEDS + pomoDuration - 1) / pomoDuration;
  int bV = 1; 

  if (currentMode == MODE_TIMER_RUN) {
    if ((millis() % 1000) < 500) bV = 1;
    else bV = 0;
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < ledsToLight) {
      if (currentMode == MODE_P12B) strip.setLedColorData(i, 0, 0, bV);
      else if (currentMode == MODE_P24Y) strip.setLedColorData(i, bV, bV, 0);
      else if (currentMode == MODE_P36R) strip.setLedColorData(i, bV, 0, 0);
      else if (currentMode == MODE_P6G) strip.setLedColorData(i, 0, bV, 0);
      else if (currentMode == MODE_TIMER_RUN) strip.setLedColorData(i, bV, 0, bV); 
    } else {
      strip.setLedColorData(i, 0, 0, 0);
    }
  }
  strip.show();
}

void updateClock() {
  struct tm ti;
  if (!getLocalTime(&ti)) return;
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
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < customSetLeds) strip.setLedColorData(i, 2, 0, 2); 
    else if (i == customSetLeds) {
       int pulse = (millis() % 1000 < 500) ? 1 : 0;
       strip.setLedColorData(i, pulse, 0, pulse);
    } else strip.setLedColorData(i, 0, 0, 0);
  }
  strip.show();
}

String getHTML() {
  String statusText = "未知狀態";
  String colorHex = "#ffffff";
  bool showTimer = false;

  switch(currentMode) {
    case MODE_OFF: statusText = "已熄滅"; colorHex = "#333"; break;
    case MODE_CLOCK: statusText = "🕒 網路時鐘"; colorHex = "#0ff"; break;
    case MODE_P12B: statusText = "🔵 12m 專注"; colorHex = "#0044ff"; showTimer = true; break;
    case MODE_P24Y: statusText = "🟡 24m 專注"; colorHex = "#aa8800"; showTimer = true; break;
    case MODE_P36R: statusText = "🔴 36m 專注"; colorHex = "#aa0000"; showTimer = true; break;
    case MODE_P6G: statusText = "🟢 6m 休息"; colorHex = "#008800"; showTimer = true; break;
    case MODE_TIMER_SET: statusText = "💜 設定計時中..."; colorHex = "#ff00ff"; break;
    case MODE_TIMER_RUN: statusText = "⌛ 自定義倒數中"; colorHex = "#ff00ff"; showTimer = true; break;
    default: break;
  }

  String html = "<!DOCTYPE html><html lang='zh-TW'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Pomo-Clock Dashboard</title>";
  html += "<style>body{font-family:'Segoe UI',Roboto,sans-serif;background:#0d1117;color:#c9d1d9;padding:20px;margin:0;display:flex;flex-direction:column;align-items:center;}";
  html += ".card{background:#161b22;border:1px solid #30363d;border-radius:15px;padding:25px;width:100%;max-width:350px;box-shadow:0 10px 30px rgba(0,0,0,0.5);margin-bottom:20px;}";
  html += ".status-badge{display:inline-block;padding:5px 15px;border-radius:20px;font-size:0.9em;font-weight:bold;margin-top:10px;background:" + colorHex + "33;color:" + colorHex + ";border:1px solid " + colorHex + "66;}";
  html += ".time-display{font-size:3.5em;font-weight:bold;margin:20px 0;letter-spacing:-2px;color:" + colorHex + ";text-shadow:0 0 20px " + colorHex + "66;}";
  html += ".btn-grid{display:grid;grid-template-columns:1fr 1fr;gap:10px;width:100%;max-width:350px;}";
  html += ".btn{padding:15px;border:none;border-radius:10px;color:white;font-weight:bold;cursor:pointer;text-decoration:none;transition:0.2s;display:flex;align-items:center;justify-content:center;font-size:0.9em;}";
  html += ".btn:active{transform:scale(0.95);}";
  html += ".p1{background:#0044ff;}.p2 {background:#aa8800;}.p3 {background:#aa0000;}.p4 {background:#008800;}.pclock{background:#444;grid-column:span 2;}.poff{background:#222;grid-column:span 2; border:1px solid #444;}";
  html += ".input-group{margin:15px 0;display:flex;gap:5px;}.input-group input{flex:1;background:#0d1117;border:1px solid #30363d;padding:10px;color:white;border-radius:5px;}";
  html += "</style><script>setInterval(() => { if(!document.hidden) location.reload(); }, 5000);</script></head><body>";

  html += "<div class='card'>";
  html += "<h2>控制中心</h2>";
  html += "<div class='status-badge'>" + statusText + "</div>";
  
  if (currentMode == MODE_CLOCK) {
    struct tm ti; getLocalTime(&ti);
    char timeStr[20]; sprintf(timeStr, "%02d:%02d:%02d", ti.tm_hour, ti.tm_min, ti.tm_sec);
    html += "<div class='time-display'>" + String(timeStr) + "</div>";
  } else if (showTimer) {
    int m = pomoRemaining / 60; int s = pomoRemaining % 60;
    char timeStr[10]; sprintf(timeStr, "%02d:%02d", m, s);
    html += "<div class='time-display'>" + String(timeStr) + "</div>";
    int progress = (pomoRemaining * 100) / (pomoDuration > 0 ? pomoDuration : 1);
    html += "<div style='width:100%;background:#30363d;height:8px;border-radius:4px;'><div style='width:" + String(progress) + "%;background:" + colorHex + ";height:100%;border-radius:4px;transition:0.5s;'></div></div>";
  } else {
    html += "<div class='time-display'>--:--</div>";
  }
  html += "</div>";

  html += "<div class='btn-grid'>";
  html += "<a href='/clock' class='btn pclock'>🕒 顯示網路時鐘</a>";
  html += "<a href='/p12b' class='btn p1'>🔵 12m</a>";
  html += "<a href='/p24y' class='btn p2'>🟡 24m</a>";
  html += "<a href='/p36r' class='btn p3'>🔴 36m</a>";
  html += "<a href='/p6g' class='btn p4'>🟢 6m</a>";
  
  html += "<div class='card' style='grid-column:span 2;margin:10px 0;padding:15px;'>";
  html += "<div style='font-weight:bold;margin-bottom:10px;'>⌛ 遠端設定計時 (分鐘)</div>";
  html += "<form action='/custom' method='GET' class='input-group'>";
  html += "<input type='number' name='m' value='5' min='1' max='999'>";
  html += "<button type='submit' class='btn' style='background:#ff00ff;padding:10px 20px;'>啟動</button>";
  html += "</form></div>";

  html += "<a href='/off' class='btn poff'>🎬 全部熄滅 / 強制停止</a>";
  html += "</div>";

  html += "<p style='font-size:0.8em;opacity:0.5;margin-top:20px;'>Pomo-Clock v18.1 Dashboard</p></body></html>";
  return html;
}

void setup() {
  Serial.begin(115200);
  delay(1000); // 讓串口穩定
  Serial.println("\n\n====================================");
  Serial.println("  ESP32-C3 Pomo-Clock 啟動中...");
  Serial.println("====================================");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  WiFi.mode(WIFI_STA);
  Serial.print(">>> [WiFi] 正在連線至: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n>>> [WiFi] 連線成功！");
  Serial.print(">>> [Web伺服器] 請在網頁輸入以下網址：\n\n");
  Serial.print("    http://");
  Serial.println(WiFi.localIP());
  Serial.println("\n====================================\n");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println(">>> [NTP] 時間已與 pool.ntp.org 同步");
  
  server.on("/", []() { 
    server.send(200, "text/html", getHTML()); 
  });
  
  server.on("/off", []() { 
    Serial.println(">>> [Web控制] 收到關閉指令");
    ledAllOff(); 
    server.sendHeader("Location", "/"); 
    server.send(303); 
  });

  server.on("/clock", []() { 
    Serial.println(">>> [Web控制] 切換至時鐘模式");
    currentMode = MODE_CLOCK; 
    server.sendHeader("Location", "/"); 
    server.send(303); 
  });

  server.on("/p12b", []() { startTimer(12, MODE_P12B); server.sendHeader("Location", "/"); server.send(303); });
  server.on("/p24y", []() { startTimer(24, MODE_P24Y); server.sendHeader("Location", "/"); server.send(303); });
  server.on("/p36r", []() { startTimer(36, MODE_P36R); server.sendHeader("Location", "/"); server.send(303); });
  server.on("/p6g", []() { startTimer(6, MODE_P6G); server.sendHeader("Location", "/"); server.send(303); });
  
  server.on("/custom", []() {
    if (server.hasArg("m")) {
      int m = server.arg("m").toInt();
      if (m > 0) {
        Serial.printf(">>> [Web控制] 收到自定義倒數: %d 分鐘\n", m);
        startTimer(m, MODE_TIMER_RUN);
      }
    }
    server.sendHeader("Location", "/");
    server.send(303);
  });
  
  server.begin();
  strip.begin();
  ledAllOff();
  currentMode = MODE_CLOCK;
  Serial.println(">>> [系統] 啟動準備就緒，目前為時鐘模式");
}

void loop() {
  server.handleClient();
  buttonState = digitalRead(BUTTON_PIN);
  
  if (buttonState == LOW && lastButtonState == HIGH) {
    buttonDownTime = millis();
    isLongPressHandled = false;
  }
  
  if (buttonState == HIGH && lastButtonState == LOW && !isLongPressHandled) {
    Serial.print(">>> [按鈕] 短按觸發，切換模式：");
    if (currentMode == MODE_OFF) currentMode = MODE_CLOCK;
    else if (currentMode == MODE_CLOCK) startTimer(12, MODE_P12B);
    else if (currentMode == MODE_P12B) startTimer(24, MODE_P24Y);
    else if (currentMode == MODE_P24Y) startTimer(36, MODE_P36R);
    else if (currentMode == MODE_P36R) startTimer(6, MODE_P6G);
    else if (currentMode == MODE_P6G) {
      currentMode = MODE_TIMER_SET;
      customSetLeds = 1;
      lastButtonAction = millis();
      Serial.println("進入紫色設定模式");
    } else if (currentMode == MODE_TIMER_SET) {
      customSetLeds = (customSetLeds % 12) + 1;
      lastButtonAction = millis();
      Serial.printf("增加設定時間: %d 分鐘\n", customSetLeds * 5);
    } else ledAllOff();
    
    if (currentMode != MODE_TIMER_SET) Serial.println(getModeName(currentMode));
  }

  if (buttonState == LOW && !isLongPressHandled && (millis() - buttonDownTime > 1000)) {
    Serial.println(">>> [按鈕] 長按 1 秒，強制關閉 LED");
    ledAllOff();
    isLongPressHandled = true;
  }
  lastButtonState = buttonState;

  if (currentMode == MODE_TIMER_SET && (millis() - lastButtonAction > 2000)) {
    Serial.println(">>> [按鈕] 靜置 2 秒，自定義倒數啟動！");
    startTimer(customSetLeds * 5, MODE_TIMER_RUN);
  }

  if (currentMode == MODE_CLOCK) updateClock();
  else if (currentMode == MODE_TIMER_SET) updateTimerSetDisplay();
  else if (currentMode >= MODE_P12B && currentMode <= MODE_TIMER_RUN) updatePomodoro();
}