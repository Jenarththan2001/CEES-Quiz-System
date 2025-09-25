/*
  Quiz Buzzer – Arduino MEGA
  Matches PC (CheckForm) commands:
    - "BUZZ\n"           -> chirp, reply "BUZZ=OK"
    - "LEDn:x\n"         -> set team LED (n=1..5, x=0/1), reply "LEDn:x=OK"
    - "RESET\n"          -> clear lock, LEDs off, echo "RESET"

  Board sends to PC:
    - "S<team>:<option>\n" on any of the 25 button presses (e.g., "S3:D")
    - "RESET\n" when hardware reset button is pressed

  Behavior:
    - Every pushbutton press beeps the buzzer
    - First team to press in a round locks the LED for that team until RESET
*/

#include <Arduino.h>

// ---------- Pins ----------
const uint8_t BTN_PINS[5][5] = {
  {22, 23, 24, 25, 26},  // S1: A..E
  {27, 28, 29, 30, 31},  // S2
  {32, 33, 34, 35, 36},  // S3
  {37, 38, 39, 40, 41},  // S4
  {2,3,4,5,6}   // S5
};

const uint8_t RESET_PIN  = 47;                 // operator reset (to GND, INPUT_PULLUP)
const uint8_t BUZZER_PIN = 8;                  // ACTIVE buzzer (on/off)
const uint8_t LED_PINS[5] = {9,10,11,12,13}; // Team LEDs 1..5

// ---------- Timing / debounce ----------
const unsigned long DEBOUNCE_MS = 15;
const unsigned int  BUZZ_MS     = 60;

// ---------- Serial ----------
const unsigned long SERIAL_BAUD = 9600;

// ---------- State ----------
bool locked = false;    // round locked after first team press
int  lockedTeam = 0;    // 1..5, 0 = none

// Per-button debounce
bool lastStable[5][5];           // true = released (HIGH), false = pressed (LOW)
bool lastRaw[5][5];
unsigned long lastChangeMs[5][5];

// Reset button debounce
bool lastResetStable = true;
bool lastResetRaw    = true;
unsigned long lastResetChangeMs = 0;

// ---------- Helpers ----------
void ledsAllOff() {
  for (int i = 0; i < 5; i++) digitalWrite(LED_PINS[i], LOW);
}

void setTeamLed(int team, bool on) { // team 1..5
  if (team < 1 || team > 5) return;
  digitalWrite(LED_PINS[team - 1], on ? HIGH : LOW);
}

void beepMs(unsigned int ms) {
  // ACTIVE buzzer:
  digitalWrite(BUZZER_PIN, HIGH);
  delay(ms);
  digitalWrite(BUZZER_PIN, LOW);
}

void unlockRound() {
  locked = false;
  lockedTeam = 0;
  ledsAllOff();
}

void sendSchoolOption(int sIdx, int oIdx) {
  char letter = 'A' + oIdx;  // 0->A, 1->B, ...
  Serial.print('S');
  Serial.print(sIdx + 1);
  Serial.print(':');
  Serial.println(letter);
}

// Optional LED test (not used by your app right now)
void runLedCheck() {
  for (int i = 1; i <= 5; i++) { setTeamLed(i, true);  delay(120); }
  for (int i = 1; i <= 5; i++) { setTeamLed(i, false); delay(90);  }
  for (int i = 1; i <= 5; i++) setTeamLed(i, true);
  delay(150);
  for (int i = 1; i <= 5; i++) setTeamLed(i, false);
}

// ---------- Serial commands from PC ----------
void handleSerialLine(String line) {
  line.trim();
  if (!line.length()) return;

  // RESET
  if (line.equalsIgnoreCase("RESET")) {
    unlockRound();
    Serial.println("RESET");  // echo so PC sees confirmation
    return;
  }

  // LEDn:x  e.g., LED3:1
  if (line.startsWith("LED")) {
    // LEDCHECK / LEDCHK (kept for future use; not sent by your app now)
    if (line.equalsIgnoreCase("LEDCHECK") || line.equalsIgnoreCase("LEDCHK")) {
      runLedCheck();
      // no strict need to reply for your UI, but nice to have:
      Serial.println("LEDCHECK=OK");
      return;
    }

    int colon = line.indexOf(':');
    if (colon > 3) {
      int team = line.substring(3, colon).toInt();
      int val  = line.substring(colon + 1).toInt();
      setTeamLed(team, val != 0);
      Serial.print("LED");
      Serial.print(team);
      Serial.print(':');
      Serial.print(val);
      Serial.println("=OK");
      return;
    }
  }

  // BUZZ (your CheckForm sends exactly "BUZZ\n")
  if (line.equalsIgnoreCase("BUZZ")) {
    beepMs(BUZZ_MS);
    Serial.println("BUZZ=OK");
    return;
  }

  // Ignore anything else
}

// ---------- Setup ----------
void setup() {
  // Inputs (buttons) with pull-up
  for (int s = 0; s < 5; s++) {
    for (int o = 0; o < 5; o++) {
      pinMode(BTN_PINS[s][o], INPUT_PULLUP);
      lastStable[s][o] = true;   // released
      lastRaw[s][o]    = true;
      lastChangeMs[s][o] = 0;
    }
  }

  // Reset button
  pinMode(RESET_PIN, INPUT_PULLUP);
  lastResetStable = true;
  lastResetRaw    = true;

  // LEDs
  for (int i = 0; i < 5; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Serial
  Serial.begin(SERIAL_BAUD);
  Serial.println("MEGA quiz ready");
}

// ---------- Loop ----------
void loop() {
  // Read serial
  while (Serial.available() > 0) {
    String line = Serial.readStringUntil('\n'); // your PC sends '\n'
    handleSerialLine(line);
  }

  // Scan 25 individual buttons with debounce
  unsigned long now = millis();
  for (int s = 0; s < 5; s++) {
    for (int o = 0; o < 5; o++) {
      bool raw = digitalRead(BTN_PINS[s][o]);     // HIGH=released, LOW=pressed
      if (raw != lastRaw[s][o]) {
        lastRaw[s][o] = raw;
        lastChangeMs[s][o] = now;
      }
      if ((now - lastChangeMs[s][o]) >= DEBOUNCE_MS) {
        if (raw != lastStable[s][o]) {
          lastStable[s][o] = raw;
          if (raw == LOW) {
            // Pressed
            beepMs(BUZZ_MS);        // always chirp (requirement)
            sendSchoolOption(s, o); // Sx:O

            if (!locked) {
              locked = true;
              lockedTeam = s + 1;
              setTeamLed(lockedTeam, true);
            }
          }
        }
      }
    }
  }

  // Read RESET button with debounce
  bool rawReset = digitalRead(RESET_PIN); // HIGH=released, LOW=pressed
  if (rawReset != lastResetRaw) {
    lastResetRaw = rawReset;
    lastResetChangeMs = now;
  }
  if ((now - lastResetChangeMs) >= DEBOUNCE_MS) {
    if (rawReset != lastResetStable) {
      lastResetStable = rawReset;
      if (rawReset == LOW) {
        // reset pressed
        beepMs(BUZZ_MS);
        unlockRound();
        Serial.println("RESET");
      }
    }
  }
}
