// hackpad firmware v4

#include <Keyboard.h>
#include <Wire.h>
#include <WiFiNINA.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

// change these to your actual wifi so it can grab the time
const char* ssid = "your_wifi_name";
const char* pass = "your_wifi_password";

const int rowPins[3] = {5, 6, 7};
const int colPins[4] = {4, 3, 2, 1};
const int encA = 13;
const int encB = 0;
const int encBtn = 8;

#define LED_PIN A3
#define NUM_LEDS 12
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

char keymap[3][4] = {
  {'7', '8', '9', '/'},
  {'4', '5', '6', '*'},
  {'1', '2', '3', '-'}
};

bool keyState[3][4] = {false};
int lastEncA = HIGH;
int layer = 0;
bool btnWasPressed = false;
char lastKeyLabel = ' ';

unsigned long lastTimeSync = 0;
unsigned long epochAtSync = 0; // unix time from the last successful sync

void setup() {
  Serial.begin(9600);
  Keyboard.begin();

  for (int r = 0; r < 3; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], HIGH);
  }
  for (int c = 0; c < 4; c++) {
    pinMode(colPins[c], INPUT_PULLUP);
  }

  pinMode(encA, INPUT_PULLUP);
  pinMode(encB, INPUT_PULLUP);
  pinMode(encBtn, INPUT_PULLUP);

  strip.begin();
  strip.show();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("oled not found, check wiring");
    while (1);
  }

  // connect to wifi so we can get real time. if this fails clock just
  // stays at --:--:--, everything else still works fine
  WiFi.begin(ssid, pass);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    tries++;
  }
  syncTime();
  updateDisplay();
}

void syncTime() {
  if (WiFi.status() == WL_CONNECTED) {
    unsigned long t = WiFi.getTime(); // nina module handles the ntp stuff
    if (t > 0) {
      epochAtSync = t;
      lastTimeSync = millis();
    }
  }
}

// turns the synced epoch + however long its been since into HH:MM:SS
String getTimeString() {
  if (epochAtSync == 0) return "--:--:--";

  unsigned long secondsNow = epochAtSync + (millis() - lastTimeSync) / 1000;
  int h = (secondsNow % 86400L) / 3600;
  int m = (secondsNow % 3600) / 60;
  int s = secondsNow % 60;

  char buf[9];
  sprintf(buf, "%02d:%02d:%02d", h, m, s);
  return String(buf);
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(getTimeString());
  display.print("layer: ");
  display.println(layer == 0 ? "numpad" : "shortcuts");
  display.print("last: ");
  display.println(lastKeyLabel);
  display.display();
}

void sendCombo(int mod, int key) {
  Keyboard.press(mod);
  Keyboard.press(key);
  delay(20);
  Keyboard.releaseAll();
}

// runs whatever shortcut lives at this row/col on layer 1
void doShortcut(int r, int c) {
  // row0: copy, (empty), (empty), (empty)
  // row1: cut, save, undo, redo
  // row2: paste, new tab, switch window, switch desktop
  if (r == 0 && c == 0) { sendCombo(KEY_LEFT_CTRL, 'c'); lastKeyLabel = 'C'; }
  // r0c1, r0c2, r0c3 empty on purpose, nothing wired to these rn
  else if (r == 1 && c == 0) { sendCombo(KEY_LEFT_CTRL, 'x'); lastKeyLabel = 'x'; }
  else if (r == 1 && c == 1) { sendCombo(KEY_LEFT_CTRL, 's'); lastKeyLabel = 'S'; }
  else if (r == 1 && c == 2) { sendCombo(KEY_LEFT_CTRL, 'z'); lastKeyLabel = 'Z'; }
  else if (r == 1 && c == 3) { sendCombo(KEY_LEFT_CTRL, 'y'); lastKeyLabel = 'Y'; }
  else if (r == 2 && c == 0) { sendCombo(KEY_LEFT_CTRL, 'v'); lastKeyLabel = 'V'; }
  else if (r == 2 && c == 1) { sendCombo(KEY_LEFT_CTRL, 't'); lastKeyLabel = 'T'; }
  else if (r == 2 && c == 2) { sendCombo(KEY_LEFT_ALT, KEY_TAB); lastKeyLabel = 'W'; }
  else if (r == 2 && c == 3) {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press(KEY_LEFT_GUI);
    Keyboard.press(KEY_RIGHT_ARROW);
    delay(20);
    Keyboard.releaseAll();
    lastKeyLabel = 'D';
  }
}

void scanMatrix() {
  for (int r = 0; r < 3; r++) {
    digitalWrite(rowPins[r], LOW);

    for (int c = 0; c < 4; c++) {
      bool pressed = digitalRead(colPins[c]) == LOW;
      int ledIndex = r * 4 + c;

      if (pressed && !keyState[r][c]) {
        keyState[r][c] = true;
        strip.setPixelColor(ledIndex, strip.Color(0, 100, 200));
        strip.show();

        if (layer == 0) {
          Keyboard.press(keymap[r][c]);
          lastKeyLabel = keymap[r][c];
        } else {
          doShortcut(r, c);
        }
        updateDisplay();

      } else if (!pressed && keyState[r][c]) {
        keyState[r][c] = false;
        strip.setPixelColor(ledIndex, 0);
        strip.show();
        if (layer == 0) Keyboard.release(keymap[r][c]);
      }
    }

    digitalWrite(rowPins[r], HIGH);
  }
}

void checkEncoder() {
  int a = digitalRead(encA);
  if (a != lastEncA && a == LOW) {
    int b = digitalRead(encB);
    if (b != a) Keyboard.write(KEY_UP_ARROW);
    else Keyboard.write(KEY_DOWN_ARROW);
  }
  lastEncA = a;

  bool btnPressed = digitalRead(encBtn) == LOW;
  if (btnPressed && !btnWasPressed) {
    layer = 1 - layer;
    updateDisplay();
    delay(200);
  }
  btnWasPressed = btnPressed;
}

void loop() {
  scanMatrix();
  checkEncoder();

  // resync every hour so the clock doesnt slowly drift off
  if (millis() - lastTimeSync > 3600000UL) {
    syncTime();
  }

  // just refresh the clock display once a second when nothing else going on
  static unsigned long lastTick = 0;
  if (millis() - lastTick > 1000) {
    lastTick = millis();
    updateDisplay();
  }
}
