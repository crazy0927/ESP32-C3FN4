# ESP32-C3 IoT 終極番茄時鐘站 (Pomo-Clock v18.1)

[繁體中文](#繁體中文) | [English](#english) | [日本語](#日本語) | [Русский](#русский) | [Italiano](#italiano) | [Français](#français)

---

<a name="繁體中文"></a>
## 🇹🇼 繁體中文
這是一個專為深層專注與高效時間管理設計的 **ESP32-C3** 物聯網裝置。

### 🌟 核心特色
*   **🕒 網路精準時鐘**：透過 NTP 自動對時，白色(時)、青色(分)、紅色(秒)三針優雅呈現。
*   **📱 煥新 Web UI**：全新 Dark Mode 深色介面，支援即時倒數進度條與遠端設定。
*   **💜 動態計時顯示**：自定義模式下 LED 會以「一閃一閃」的呼吸感運作。

### �️ 硬體配置與接腳說明 (Hardware Pinout)

| 元件名稱 | ESP32-C3 接腳 | 說明 |
| :--- | :--- | :--- |
| **WS2812B LED 燈環** | **GPIO 10** | 數據輸入 (DIN)，建議串接 220Ω 電阻 |
| **實體按鈕 (BOOT)** | **GPIO 9** | 模式切換與設定 (內建上拉電阻) |
| **電源輸入** | **VCC (5V)** | 建議使用 5V 1A 以上電源供電 |
| **共地** | **GND** | 確保 LED 與開發板共地 |

### �📊 操作指南 (實體按鈕 GPIO 9)
*   **短按**：切換循環模式 (時鐘 > 12m > 24m > 36m > 6m > 自定義紫色模式)。
*   **紫色設定**：進入紫色模式後點擊增加時間 (5min/顆)，靜置 2 秒自動啟動。
*   **長按**：1 秒強制關閉。

---

<a name="english"></a>
## 🇺🇸 English
An ultimate **ESP32-C3** IoT device designed for deep focus and efficient time management.

### 🛠️ Hardware Connection
*   **LED Data (DIN)**: GPIO 10
*   **Button (BOOT)**: GPIO 9
*   **Power**: 5V DC

### 🌟 Key Features
*   **🕒 NTP Network Clock**: Accurate time synchronization with White(H), Cyan(M), and Red(S) hands.
*   **📱 Modern Web UI**: New Dark Mode dashboard with real-time progress bars.
*   **💜 Dynamic Visuals**: "Blinking" effect in custom timer mode.

---

<a name="日本語"></a>
## 🇯🇵 日本語
集中力向上と時間管理のための **ESP32-C3** IoT デバイスです。

### 🛠️ ハードウェア接続
*   **LED データ (DIN)**: GPIO 10
*   **ボタン (BOOT)**: GPIO 9
*   **電源**: 5V DC

### 🌟 特徴
*   **🕒 NTP 時計**: 自動時刻合わせ機能搭載。
*   **📱 モダンな Web UI**: リアルタイム進捗バー付きダッシュボード。

---

<a name="Русский"></a>
## 🇷🇺 Русский
IoT-устройство **ESP32-C3** для концентрации и управления временем.

### 🛠️ Подключение оборудования
*   **Данные LED (DIN)**: GPIO 10
*   **Кнопка (BOOT)**: GPIO 9
*   **Питание**: 5В

### 🌟 Особенности
*   **� Сетевые часы**: Точная синхронизация через NTP.

---

<a name="italiano"></a>
## 🇮🇹 Italiano
Un dispositivo IoT **ESP32-C3** definitivo per la concentrazione profonda e la gestione del tempo.

### 🛠️ Collegamenti Hardware
*   **Dati LED (DIN)**: GPIO 10
*   **Pulsante (BOOT)**: GPIO 9
*   **Alimentazione**: 5V

### 🌟 Caratteristiche Principali
*   **🕒 Orologio NTP**: Sincronizzazione precisa con lancette eleganti in Bianco(H), Ciano(M) e Rosso(S).

---

<a name="français"></a>
## 🇫🇷 Français
L'appareil IoT **ESP32-C3** ultime pour une concentration profonde et une gestion efficace du temps.

### 🛠️ Connexions Matérielles
*   **Données LED (DIN)**: GPIO 10
*   **Bouton (BOOT)**: GPIO 9
*   **Alimentation**: 5V

### 🌟 Fonctionnalités Clés
*   **🕒 Horloge NTP**: Synchronisation précise avec des aiguilles élégantes : Blanc(H), Cyan(M) et Rouge(S).

---
**Version: v18.1 [World-Edition]**  
*Developed with Antigravity. Built for the global developer community.*
