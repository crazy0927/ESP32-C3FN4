# ESP32-C3 WS2812 LED 控制開發紀錄 (2025-12-25)

## 1. 專案背景
在本專案中，我們嘗試在 **ESP32-C3** 開發板上透過新版 **Arduino ESP32 Core 3.x** 驅動 WS2812B RGB LED 燈條。

## 2. 核心問題與測試過程
今天我們經歷了漫長的調試過程，主要挑戰在於 ESP32-C3 的 WiFi 功能與 LED 訊號時脈之間的衝突。

### 階段一：Adafruit NeoPixel 的挫敗 (錯誤多次)
*   **現象**：LED 無法穩定熄滅，或是點亮後出現亂碼（隨機顏色閃爍）。
*   **探究原因**：
    *   `Adafruit_NeoPixel` 使用的是 bit-banging 技術（軟體模擬時序）。
    *   在 ESP32-C3 Core 3.x 中，背景系統任務（如 WiFi）會中斷軟體計時，導致發送給 LED 的 0/1 訊號變形。
*   **嘗試過的補償方案**：
    *   強制關閉 WiFi。
    *   連續發送 5 次 `show()` 脈衝（Step 40 策略）。
    *   調整系統時脈設定。
    *   *結論*：雖然有改善，但依然不夠完美。

### 階段二：轉向 Freenove RMT 方案 (成功關鍵)
*   **切換函式庫**：引入 `Freenove_WS2812_Lib_for_ESP32`。
*   **成功原因**：
    *   此函式庫使用 ESP32 內建的 **RMT (Remote Control Peripheral)** 硬體模組。
    *   RMT 是硬體級的發射器，擁有獨立的緩衝區與時脈，不會被 CPU 的 WiFi 任務中斷。
*   **最終結果**：**完全控制**。指令 `0` (熄滅)、`1` (點亮)、`2` (彩虹) 均能 100% 精準執行。

---

## 3. 程式碼分析 (WIFI_Rainbow_RGB.ino)

### 關鍵宣告
```cpp
#include "Freenove_WS2812_Lib_for_ESP32.h"
#define CHANNEL 0 // 使用 RMT 硬體通道 0
Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(NUM_LEDS, LED_PIN, CHANNEL, TYPE_GRB);
```
這段決定了使用硬體驅動而非軟體模擬。

### 功能模組
1.  **WiFi 初始化**：
    ```cpp
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    ```
    即便開啟 WiFi，LED 訊號依然穩定。
2.  **指令系統**：
    *   `'1'`：靜態綠色。
    *   `'0'`：全滅。
    *   `'2'`：動態彩虹 Flag 開關。
3.  **彩虹演算法**：
    透過 `strip.Wheel()` 配合不斷累積的 `rainbowStep` 實現色彩循環，並在 `loop()` 中以 `delay(10)` 維持動畫流暢度。

---

## 4. 總結與心得
*   **硬體驅動 > 軟體模擬**：在 ESP32 等多任務系統上，優先選擇 RMT 硬體驅動。
*   **亮度控制**：設定 `BRIGHTNESS 10` 既美觀又保護硬體。
*   **今日結論**：經歷了 Adafruit 函式庫多次失敗的震撼教育後，最終確認 **Freenove RMT** 是 ESP32-C3 LED 控制的終極解答。

---
**紀錄日期**：2025-12-25  
**狀態**：已更新至 GitHub (v14.0)
