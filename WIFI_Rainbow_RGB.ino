#include <WiFi.h>          // 引入 WiFi 功能函式庫
#include <WebServer.h>     // 引入網頁伺服器功能函式庫
#include "Freenove_WS2812_Lib_for_ESP32.h" // 引入 LED 驅動函式庫 (針對 ESP32 優化)
#include "secrets.h"       // 引入私密資訊 (WiFi 名稱與密碼)

// --- 專案版本與標籤 ---
const char* TAG = "v15.2 [IoT LED Controller]";

// --- WiFi 設定 (從 secrets.h 讀取，保護後台資訊) ---
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

// --- 硬體接腳與參數設定 ---
#define LED_PIN     10    // LED 連接到開發板的第 10 號接腳
#define NUM_LEDS    12    // LED 燈環的總燈數 (12 顆)
#define CHANNEL     0     // ESP32 RMT 訊號通道
#define BRIGHTNESS  10    // 亮度設定 (最小 0, 最大 255)

// --- 初始化物件 ---
// 建立 LED 控制物件
Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(NUM_LEDS, LED_PIN, CHANNEL, TYPE_GRB);
// 建立網頁伺服器物件，使用 80 連接埠 (標準 HTTP 埠)
WebServer server(80);

// --- 系統變數 ---
enum LEDMode { 
  MODE_OFF,     // 熄滅模式
  MODE_RAINBOW, // 彩虹模式
  MODE_CW,      // 順時針跑馬燈 (Clockwise)
  MODE_CCW      // 逆時針跑馬燈 (Counter-Clockwise)
};

LEDMode currentMode = MODE_OFF; // 一開始預設為熄滅
uint8_t rainbowStep = 0;        // 彩虹顏色的偏移值
int marqueeStep = 0;            // 跑馬燈目前的燈位置
unsigned long lastUpdate = 0;   // 記錄上次更新動畫的時間 (用於非阻塞更新)

// --- LED 控制函式庫 ---

// 函式：將所有燈點亮為綠黃色，並停止動畫
void ledAllOn() {
  currentMode = MODE_OFF;
  for (int i = 0; i < NUM_LEDS; i++) strip.setLedColor(i, 0, 255, 100);
  strip.show();
  Serial.println("[LED] 全部點亮");
}

// 函式：徹底熄滅所有燈，並停止動畫
void ledAllOff() {
  currentMode = MODE_OFF;
  for (int i = 0; i < NUM_LEDS; i++) strip.setLedColor(i, 0, 0, 0);
  strip.show();
  Serial.println("[LED] 全部熄滅");
}

// 函式：切換目前的模式並重置跑馬燈計數
void setMode(LEDMode m) {
  currentMode = m;
  marqueeStep = 0; // 重置動畫位置
  Serial.print("[系統] 切換模式為: ");
  if (m == MODE_RAINBOW) Serial.println("彩虹效果");
  else if (m == MODE_CW) Serial.println("順時針跑馬燈");
  else if (m == MODE_CCW) Serial.println("逆時針跑馬燈");
}

// --- 網頁 HTML 介面生成 ---
String getHTML() {
  String statusStr = "靜態/熄滅";
  if (currentMode == MODE_RAINBOW) statusStr = "彩虹效果中";
  else if (currentMode == MODE_CW) statusStr = "順時針跑馬燈中";
  else if (currentMode == MODE_CCW) statusStr = "逆時針跑馬燈中";

  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32-C3 LED 控制中心</title>";
  html += "<style>";
  html += "body { font-family: 'Segoe UI', Arial, sans-serif; text-align: center; background: #222; color: #fff; padding: 20px; }";
  html += "h1 { color: #00e5ff; }";
  html += ".status { font-size: 1.2em; color: #ffeb3b; margin-bottom: 20px; }";
  html += ".btn { display: block; width: 80%; max-width: 300px; margin: 15px auto; padding: 15px; font-size: 18px; color: white; border: none; border-radius: 50px; cursor: pointer; text-decoration: none; transition: 0.3s; }";
  html += ".on { background: #4CAF50; } .off { background: #f44336; } .rainbow { background: linear-gradient(to right, #f12711, #f5af19); }";
  html += ".cw { background: #2196F3; } .ccw { background: #9C27B0; }";
  html += ".btn:hover { filter: brightness(1.2); transform: scale(1.05); }";
  html += ".footer { margin-top: 30px; font-size: 0.8em; color: #888; }";
  html += "</style></head><body>";
  
  html += "<h1>ESP32-C3 控制中心</h1>";
  html += "<div class='status'>當前狀態: " + statusStr + "</div>";
  
  // 建立按鈕連結
  html += "<a href='/on' class='btn on'>全部點亮</a>";
  html += "<a href='/off' class='btn off'>全部熄滅</a>";
  html += "<a href='/rainbow' class='btn rainbow'>彩虹模式</a>";
  html += "<a href='/cw' class='btn cw'>順時針跑馬燈</a>";
  html += "<a href='/ccw' class='btn ccw'>逆時針跑馬燈</a>";
  
  html += "<div class='footer'>系統版本: " + String(TAG) + "</div>";
  html += "</body></html>";
  return html;
}

// --- 初始化程序 (只執行一次) ---
void setup() {
  Serial.begin(115200); // 啟動序列埠通訊，波特率 115200
  delay(1000);
  
  Serial.println("\n-------------------------------------------");
  Serial.println(TAG);
  Serial.println("-------------------------------------------");

  // --- 連接 WiFi ---
  WiFi.mode(WIFI_STA); // 設定為基地台連線模式
  WiFi.begin(ssid, password);
  Serial.print("正在連線到 WiFi");
  
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(500);
    Serial.print(".");
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[成功] WiFi 已連線");
    Serial.print("網頁控制 IP 位址: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[失敗] WiFi 無法連線，將僅使用串列控制");
  }

  // --- 設定網頁路徑 ---
  server.on("/", []() { server.send(200, "text/html", getHTML()); });
  server.on("/on", []() { ledAllOn(); server.sendHeader("Location", "/"); server.send(303); });
  server.on("/off", []() { ledAllOff(); server.sendHeader("Location", "/"); server.send(303); });
  server.on("/rainbow", []() { setMode(MODE_RAINBOW); server.sendHeader("Location", "/"); server.send(303); });
  server.on("/cw", []() { setMode(MODE_CW); server.sendHeader("Location", "/"); server.send(303); });
  server.on("/ccw", []() { setMode(MODE_CCW); server.sendHeader("Location", "/"); server.send(303); });
  
  server.begin(); // 啟動網頁伺服器

  // --- 初始化 LED 燈環 ---
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  ledAllOff(); // 初始狀態設為熄滅

  Serial.println("\n系統已就緒，請開啟網頁進行操作！");
}

// --- 彩虹效果邏輯 ---
void updateRainbow() {
  for (int i = 0; i < NUM_LEDS; i++) {
    // 根據目前的偏移值計算每顆燈的顏色
    strip.setLedColorData(i, strip.Wheel((i * 256 / NUM_LEDS + rainbowStep) & 255));
  }
  strip.show();
  rainbowStep += 2; // 增加偏移值，讓顏色產生流動感
}

// --- 順時針跑馬燈邏輯 ---
void updateClockwise() {
  for (int i = 0; i < NUM_LEDS; i++) strip.setLedColorData(i, 0, 0, 0);
  strip.setLedColorData(marqueeStep, 0, 255, 255); // 顯示青色
  strip.show();
  marqueeStep = (marqueeStep + 1) % NUM_LEDS; // index 加一並循環
}

// --- 逆時針跑馬燈邏輯 ---
void updateCounterClockwise() {
  for (int i = 0; i < NUM_LEDS; i++) strip.setLedColorData(i, 0, 0, 0);
  strip.setLedColorData(marqueeStep, 255, 0, 255); // 顯示粉紫色
  strip.show();
  marqueeStep--;
  if (marqueeStep < 0) marqueeStep = NUM_LEDS - 1; // 循環回到最後一顆
}

// --- 主循環 (重複執行) ---
void loop() {
  server.handleClient(); // 處理網頁上的使用者操作

  // 處理序列埠指令 (1=亮, 0=滅, 2=彩虹, 3=順時, 4=逆時)
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == '1') ledAllOn();
    else if (cmd == '0') ledAllOff();
    else if (cmd == '2') setMode(MODE_RAINBOW);
    else if (cmd == '3') setMode(MODE_CW);
    else if (cmd == '4') setMode(MODE_CCW);
  }

  // --- 非阻塞控制動畫更新 ---
  // 使用 millis() 判斷時間而非 delay()，這樣才不會卡住網頁伺服器的反應
  unsigned long now = millis();
  int interval = (currentMode == MODE_RAINBOW) ? 15 : 100; // 彩虹快一點，跑馬燈慢一點
  
  if (currentMode != MODE_OFF && (now - lastUpdate > interval)) {
    lastUpdate = now;
    if (currentMode == MODE_RAINBOW) updateRainbow();
    else if (currentMode == MODE_CW) updateClockwise();
    else if (currentMode == MODE_CCW) updateCounterClockwise();
  }
}