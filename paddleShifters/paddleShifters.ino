#include <Joystick.h>

// -------------------------
// Paddle switch pins
// -------------------------
const uint8_t DOWNSHIFT_PIN = 4;
const uint8_t UPSHIFT_PIN   = 2;

// Create a USB game controller:
// - 2 buttons
// - 0 hat switches
// - no analog axes
Joystick_ Joystick(
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_GAMEPAD,
  2,      // Number of buttons
  0,      // Number of hat switches

  false,  // X axis
  false,  // Y axis
  false,  // Z axis
  false,  // Rx axis
  false,  // Ry axis
  false,  // Rz axis
  false,  // Rudder
  false,  // Throttle
  false,  // Accelerator
  false,  // Brake
  false   // Steering
);

// -------------------------
// Debounce variables
// -------------------------
bool lastRawDown = HIGH;
bool stableDown  = HIGH;

bool lastRawUp = HIGH;
bool stableUp  = HIGH;

unsigned long downDebounceTime = 0;
unsigned long upDebounceTime   = 0;

const unsigned long DEBOUNCE_MS = 10;

void setup() {
  // Internal pull-up resistors
  pinMode(DOWNSHIFT_PIN, INPUT_PULLUP);
  pinMode(UPSHIFT_PIN, INPUT_PULLUP);

  // Start USB game controller
  Joystick.begin();
}

void loop() {

  // ========================
  // DOWNSHIFT PADDLE
  // ========================
  bool rawDown = digitalRead(DOWNSHIFT_PIN);

  if (rawDown != lastRawDown) {
    downDebounceTime = millis();
    lastRawDown = rawDown;
  }

  if ((millis() - downDebounceTime) > DEBOUNCE_MS) {
    if (rawDown != stableDown) {
      stableDown = rawDown;

      // Button 0 in code = Button 1 in Windows
      Joystick.setButton(0, stableDown == LOW);
    }
  }

  // ========================
  // UPSHIFT PADDLE
  // ========================
  bool rawUp = digitalRead(UPSHIFT_PIN);

  if (rawUp != lastRawUp) {
    upDebounceTime = millis();
    lastRawUp = rawUp;
  }

  if ((millis() - upDebounceTime) > DEBOUNCE_MS) {
    if (rawUp != stableUp) {
      stableUp = rawUp;

      // Button 1 in code = Button 2 in Windows
      Joystick.setButton(1, stableUp == LOW);
    }
  }
}