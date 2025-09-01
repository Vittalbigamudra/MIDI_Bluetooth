#include <BluetoothSerial.h>
#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32.h>


BLEMIDI_CREATE_DEFAULT_INSTANCE();

// -----------------------
// CONFIGURATION
// -----------------------

// Piano button matrix
const int ROWS = 4;  // e.g. 4 rows
const int COLS = 8;  // e.g. 8 columns = 32 keys total
int rowPins[ROWS] = {13, 12, 14, 27};   // adjust for your wiring
int colPins[COLS] = {26, 25, 33, 32, 19, 18, 5, 4};

// Potentiometer (expression / modulation)
const int POT_PIN = 34;

// Base MIDI note (C3 for example)
const int BASE_NOTE = 48; // MIDI note number for C3

// State tracking
bool keyState[ROWS][COLS] = {false};
int lastPotValue = -1;

void setup() {
  Serial.begin(115200);

  // Setup matrix pins
  for (int r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], HIGH);
  }
  for (int c = 0; c < COLS; c++) {
    pinMode(colPins[c], INPUT_PULLUP);
  }

  // BLE MIDI
  BLEMIDI.begin("ESP32_Piano");
  Serial.println("Bluetooth MIDI Piano ready!");
}

void loop() {
  scanKeys();
  readPot();
}

// -----------------------
// MATRIX SCAN
// -----------------------
void scanKeys() {
  for (int r = 0; r < ROWS; r++) {
    digitalWrite(rowPins[r], LOW); // activate row
    delayMicroseconds(5);

    for (int c = 0; c < COLS; c++) {
      bool pressed = (digitalRead(colPins[c]) == LOW);

      if (pressed != keyState[r][c]) {
        int note = BASE_NOTE + (r * COLS + c); // map button to note
        if (pressed) {
          BLEMIDI.noteOn(1, note, 100);
          Serial.printf("Note On: %d\n", note);
        } else {
          BLEMIDI.noteOff(1, note, 0);
          Serial.printf("Note Off: %d\n", note);
        }
        keyState[r][c] = pressed;
      }
    }

    digitalWrite(rowPins[r], HIGH); // deactivate row
  }
}

// -----------------------
// POTENTIOMETER CONTROL
// -----------------------
void readPot() {
  int val = analogRead(POT_PIN) / 8; // 0-4095 -> 0-511
  val = constrain(val, 0, 127);

  if (val != lastPotValue) {
    BLEMIDI.controlChange(1, 1, val); // CC1 = Mod Wheel
    Serial.printf("CC1: %d\n", val);
    lastPotValue = val;
  }
}
