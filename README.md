# ESP32-C3 IoT 終極番茄時鐘站 (Pomo-Clock v18.1)

[繁體中文](#繁體中文) | [English](#english) | [日本語](#日本語) | [Русский](#русский) | [Italiano](#italiano) | [Français](#français)

---

<a name="繁體中文"></a>
## 🇹🇼 繁體中文
這是一個專為深層專注與高效時間管理設計的 **ESP32-C3** 物聯網裝置。

### 🌟 核心特色
*   **🕒 網路精準時鐘**：透過 **`pool.ntp.org`** 自動對時，白色(時)、青色(分)、紅色(秒)三針優雅呈現。
*   **📱 煥新 Web UI**：全新 Dark Mode 深色介面，支援即時倒數進度條與遠端設定。
*   **💜 動態計時顯示**：自定義模式下 LED 會以「一閃一閃」的呼吸感運作。

### � 使用情境 (Use Cases)
*   **💻 程式開發/讀書專注**：利用固定 12/24/36 分鐘番茄鐘，進入全心投入狀態。
*   **🍵 生活計時**：泡茶 (3min)、泡麵 (5min)、午睡 (20min) 均可透過「自定義模式」快速設定。
*   **🛌 夜間時鐘**：極低亮度設計，放在床頭作為時鐘使用也不會影響睡眠質量。

### �🛠️ 硬體接腳說明 (Hardware Pinout)
*   **LED 數據線 (DIN)**: **GPIO 10**
*   **實體按鈕 (BOOT)**: **GPIO 9** (模式切換)
*   **電源**: 5V DC 至 VCC/GND

### � 操作流程說明 (Workflows)

#### 1. 實體按鈕操作 (Manual Button)
*   **循環預設模式**：短按按鈕可切換模式，順序為：`時鐘` -> `12分` -> `24分` -> `36分` -> `6分休息` -> `進入自定義設定`。
*   **自定義計時器設定**：
    1. 切換至**紫色模式**（最後一顆燈會閃爍）。
    2. 每按一下增加 5 分鐘（多亮一顆燈）。
    3. **靜置 2 秒**，裝置會閃爍紫色光並自動開始倒數。
*   **強制結束**：在任何倒數過程中「短按」一下即可回到關閉模式，或「長按 1 秒」關閉所有燈光。

#### 2. 網頁遠端操作 (Web Dashboard)
1.  **連線**：開啟序列埠 (Serial Monitor) 查看 IP 地址（例如 `http://192.168.x.x`）。
2.  **儀表板控制**：
    *   點擊按鈕直接啟動固定時間的番茄鐘。
    *   在 **「遠端設定計時」** 輸入框輸入任意分鐘（例如 45），按下「啟動」後開發板會立即同步紫色倒數效果。
3.  **即時監控**：網頁會主動顯示當前剩餘的分秒數與進度條，每 5 秒自動整理狀態。

#### 3. 時鐘同步邏輯 (Clock Sync)
*   本裝置啟動後會自動向 `pool.ntp.org` 發送請求，並根據 `GMT+8` (台灣時區) 校正本地時間。

---

<a name="english"></a>
## 🇺🇸 English

### 🌟 Key Features
*   **🕒 High-Accuracy NTP Clock**: Synchronized via **`pool.ntp.org`**.
*   **📖 Scenarios**: Deep work, meditation, cooking timer, or dim bedside clock.

### 🔄 Workflows
*   **Button**: Click to cycle modes. In **Purple Mode**, click to add time (+5min), wait 2s to start.
*   **Web UI**: Access the local IP (e.g., `http://192.168.x.x`) to control timers and monitor progress bars.

---

<a name="日本語"></a>
## 🇯🇵 日本語

### � 操作フロー
*   **物理ボタン**: クリックでモード切替。**紫色モード**中に時間を追加（5分/灯）、2秒間待つと自動的にカウントダウンが始まります。
*   **Web ダッシュボード**: シリアルモニタで IP アドレスを確認し、ブラウザからリモート操作や進捗確認が可能です。

---

<a name="Русский"></a>
## 🇷🇺 Русский

### 🔄 Инструкция
*   **Кнопка**: Переключение режимов. В **Пурпурном режиме** нажимайте для добавления времени (+5 мин), подождите 2 сек для запуска.
*   **Web-интерфейс**: Управление через локальный IP-адрес с отображением прогресса в реальном времени.

---

<a name="italiano"></a>
## 🇮🇹 Italiano

### 🔄 Workflow
*   **Pulsante**: Ciclo modalità. In **Modalità Viola**, premi per aggiungere tempo (+5min), attendi 2s per l'avvio.
*   **Web UI**: Controlla i timer e monitora i progressi tramite l'IP locale nel browser.

---

<a name="français"></a>
## 🇫🇷 Français

### 🔄 Flux de Travail
*   **Bouton**: Cycle des modes. En **Mode Violet**, cliquez pour ajouter du temps (+5 min), attendez 2 s pour démarrer.
*   **Web UI**: Accédez à l'IP locale pour piloter les minuteries et suivre la barre de progression.

---
**Version: v18.1 [Documented-World-Edition]**  
*Developed with Antigravity. High-performance time management logic.*
