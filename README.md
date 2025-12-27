# ESP32-C3 IoT 終極番茄時鐘站 (Pomo-Clock v18.1)

[繁體中文](#繁體中文) | [English](#english) | [日本語](#日本語) | [Русский](#русский)

---

<a name="繁體中文"></a>
## 🇹🇼 繁體中文
這是一個專為深層專注與高效時間管理設計的 **ESP32-C3** 物聯網裝置。

### 🌟 核心特色
*   **🕒 網路精準時鐘**：透過 NTP 自動對時，白色(時)、青色(分)、紅色(秒)三針優雅呈現。
*   **📱 煥新 Web UI**：全新 Dark Mode 深色介面，支援即時倒數進度條與遠端設定。
*   **💜 動態計時顯示**：自定義模式下 LED 會以「一閃一閃」的呼吸感運作。

### 📊 操作指南 (實體按鈕 GPIO 9)
*   **短按**：切換循環模式 (時鐘 > 12m > 24m > 36m > 6m > 自定義紫色模式)。
*   **紫色設定**：進入紫色模式後點擊增加時間 (5min/顆)，靜置 2 秒自動啟動。
*   **長按**：1 秒強制關閉。

---

<a name="english"></a>
## 🇺🇸 English
An ultimate **ESP32-C3** IoT device designed for deep focus and efficient time management.

### 🌟 Key Features
*   **🕒 NTP Network Clock**: Accurate time synchronization with elegant White(H), Cyan(M), and Red(S) hands.
*   **📱 Modern Web UI**: New Dark Mode dashboard featuring real-time progress bars and remote timer configuration.
*   **� Dynamic Visuals**: "Blinking" effect in custom timer mode to distinguish it from steady clock/pomodoro modes.

### 📊 Operation Guide (Button GPIO 9)
*   **Short Press**: Cycle modes (Clock > 12m > 24m > 36m > 6m > Custom Purple Mode).
*   **Purple Setup**: Click to add time (+5min/LED) when in purple mode. Wait 2s to start automatically.
*   **Long Press**: 1s to force turn off all LEDs.

---

<a name="日本語"></a>
## 🇯🇵 日本語
深い集中と効率的な時間管理のために設計された **ESP32-C3** IoT デバイスです。

### 🌟 主な機能
*   **🕒 NTP ネットワーク時計**: NTP による自動時刻合わせ。白(時)、シアン(分)、赤(秒)の優雅な表示。
*   **📱 新しい Web UI**: ダークモード対応のダッシュボード。リアルタイム進捗バーとリモートタイマー設定をサポート。
*   **💜 動的な表示**: カスタムタイマーモードでは LED が「点滅」し、集中モードと区別されます。

### 📊 操作ガイド (物理ボタン GPIO 9)
*   **短押し**: モード切替 (時計 > 12分 > 24分 > 36分 > 6分 > カスタム紫モード)。
*   **紫モード設定**: 紫モード中にクリックで時間を追加 (各LED+5分)。2秒間静止で自動開始。
*   **長押し**: 1秒押しで LED を強制消灯。

---

<a name="Русский"></a>
## 🇷🇺 Русский
Максимальное IoT-устройство на базе **ESP32-C3** для глубокой концентрации и эффективного управления временем.

### 🌟 Основные функции
*   **🕒 Сетевые часы NTP**: Автоматическая синхронизация времени с элегантным отображением: белый (час), голубой (мин) и красный (сек).
*   **📱 Современный Web-интерфейс**: Новая темная тема с индикатором прогресса в реальном времени и удаленной настройкой таймера.
*   **💜 Динамическая индикация**: Эффект «мигания» в пользовательском режиме таймера для отличия от режима часов.

### � Руководство (Кнопка GPIO 9)
*   **Короткое нажатие**: Циклическое переключение (Часы > 12м > 24м > 36м > 6м > Пурпурный режим).
*   **Пурпурная настройка**: В пурпурном режиме нажимайте для добавления времени (+5 мин на LED). Подождите 2 сек для запуска.
*   **Длительное нажатие**: 1 сек для принудительного выключения всех светодиодов.

---
**Version: v18.1 [Multilingual-Release]**  
*Developed with Antigravity. Built for the ultimate desktop workspace.*
