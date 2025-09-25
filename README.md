# CEES Quiz System 🎉

**SCIENCE AND TECHNOLOGY – QUIZ COMPETITION 2025**


*Civil and Environmental Engineering Society, University of Jaffna*

A complete buzzer + scoring solution:

* ⚡ **Arduino Mega** hardware that captures button presses, locks the first team, and controls team LEDs.
* 💻 **Windows desktop app** (Telerik WinForms) for timing, reveal, serial parsing, scoring, and CSV export.
* 🌐 **Web “Podium”** (2 static pages) to enter schools and scores and display full-screen results (bar chart + table).

---

## 📸 Final Quiz Captures

<div align="center">
  <img src="assets/1.jpg" width="700" style="margin:10px;"/>
  <img src="assets/2.jpg" width="700" style="margin:10px;"/>
  <img src="assets/3.jpg" width="700" style="margin:10px;"/>
  <img src="assets/4.jpg" width="700" style="margin:10px;"/>
  <img src="assets/5.jpg" width="700" style="margin:10px;"/>
  <img src="assets/6.jpg" width="700" style="margin:10px;"/>
  <img src="assets/7.jpg" width="700" style="margin:10px;"/>
  <img src="assets/8.jpg" width="700" style="margin:10px;"/>
</div>

---

## 🏗️ Architecture Overview

**Flow:**

1. Contestant presses a button → **Arduino Mega** debounces input → locks first team → turns that team LED **ON**.
2. Arduino sends a **serial line** to PC, e.g. `S2:B` (Team 2 chose B) or `RESET`.
3. **Windows app** parses serial, starts/stops timer, stores answers, and maintains live leaderboard.
4. At the end, scores are pushed to the **Podium**: a clean, full-screen web page (Chart.js bar chart + ranking table).

---

## 🔌 Hardware (Embedded)

* **MCU:** Arduino Mega 2560 (lots of GPIO)
* **Buttons:** 5 teams × 5 options (A–E) = 25 inputs
* **Operator Reset:** 1 input
* **LEDs:** 5 outputs (one per team)
* **Buzzer:** optional “chirp” on press
* **Power:** USB or external 9–12 V

**Firmware Features**

* Debounce for all 26 buttons
* **Lockout** after the first press per question
* **Serial** at 9600 baud
* Commands **from** PC:

  * `RESET` → unlock LEDs and round, echo `RESET`
  * `LED<n>:0|1` → set team LED
  * `BUZZ` → chirp buzzer (test)
* Events **to** PC:

  * `S<team>:<option>` (e.g. `S3:D`)
  * `RESET` (if reset button pressed)

📂 Source: `arduino/Final_Code_Quiz.ino`

---

## 💻 Windows Desktop App

* **Tech:** C# (.NET) + Telerik WinForms
* **Features:**

  * Connect to COM port, read serial lines
  * 1-second **countdown timer**; auto-reveal at time-up
  * **Reveal** toggle (hide/show options)
  * **Leaderboard grid** (school, option, time)
  * **CSV export** for all captured answers
  * **“Podium” bar chart** (gold/silver/bronze top 3)

**Install/Run:**

* Download from [Releases](../../releases)
  or use files in `win-desktop/`: `CEES Quiz.msi`, `setup.exe`, `CEES Quiz.zip`

---

## 🌐 Web Podium (2 pages)

A zero-backend, browser-only solution that runs locally or via **GitHub Pages**.

* `web-podium/index.html`

  * Enter school names & scores (2–5 schools)
  * **Display Results** → saves to `localStorage` and opens results
  * **Clear All** resets form
* `web-podium/results.html`

  * Bar chart (Chart.js) + ranking table
  * **Fullscreen** button
  * **Back** to modify entries

👉 If GitHub Pages is enabled, your live podium will be available at:

```
https://jenarththan2001.github.io/CEES-Quiz-System/
```

---

## 🔄 Serial Protocol

* `S<team>:<option>` → event (first press per team locks)
  Example: `S2:B`
* `RESET` → sent by device when reset button pressed; also echoed after PC sends `RESET`
* `LED<n>:0|1` (PC → device) → turn a team LED off/on (e.g. `LED3:1`)

---

## ▶️ How to Run

### A) Hardware + Windows App

1. Upload `arduino/Final_Code_Quiz.ino` sketch to **Arduino Mega**.
2. Wire 25 answer buttons, reset button, 5 LEDs, and buzzer.
3. Start the **Windows app**, select COM port, and connect.
4. Use **Reset** to open a question round; teams press once; results recorded.
5. Export CSV or run the **Web Podium** to present final scores.

### B) Web Podium Only

* Open `web-podium/index.html` (or GitHub Pages URL)
* Enter schools + scores → **Display Results**
* Present `web-podium/results.html` in fullscreen

---

## 📂 Folder Structure

```
arduino/
  Final_Code_Quiz.ino

win-desktop/
  CEES Quiz.msi
  setup.exe
  CEES Quiz.zip

web-podium/
  index.html
  results.html

assets/
  1.jpg ... 8.jpg
  ds_digital.zip
```

---

## ⚙️ Build / Dev Notes

* **WinForms (Telerik):** requires Telerik UI for WinForms in Visual Studio.
* **Charts:**

  * Desktop: Telerik RadChartView
  * Web: Chart.js (CDN)
* **Data persistence:**

  * Desktop: in-memory (CSV export available)
  * Web: `localStorage`

---

## 📜 License

MIT — see [LICENSE](./LICENSE)

---

<div align="center">
  <b>✨ CEES Quiz System — Bringing fair play & excitement to science competitions ✨</b>
</div>
