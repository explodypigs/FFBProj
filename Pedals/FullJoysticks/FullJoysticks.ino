#include <HX711.h>
#include <Joystick.h>

// =====================================================
// PIN CONFIGURATION
// =====================================================

// 49E Hall-effect throttle
const int THROTTLE_PIN = A2;

// HX711 load cell
const int HX711_DOUT = 2;   // DT  -> D2
const int HX711_SCK  = 4;   // SCK -> D4

HX711 brakeScale;


// =====================================================
// THROTTLE CALIBRATION
// =====================================================

const int THROTTLE_RELEASED = 527;
const int THROTTLE_PRESSED  = 810;

const int THROTTLE_DEADZONE = 3;


// =====================================================
// BRAKE CALIBRATION
// =====================================================

// Brake automatically tares at startup.
//
// Idle noise is only a few hundred counts,
// so anything above -2000 is considered released.

const long BRAKE_START = -2000;

// Your measured maximum brake force
const long BRAKE_FULL = -350000;


// =====================================================
// BRAKE FILTER
// =====================================================

long filteredBrake = 0;

bool brakeFilterInitialized = false;


// =====================================================
// DEBUG / SERIAL MONITOR
// =====================================================

// Print diagnostic data every 100 ms.
//
// IMPORTANT:
// We do NOT print every loop because Serial output
// could slow down the pedal controller.

unsigned long lastDebugPrint = 0;

const unsigned long DEBUG_INTERVAL = 100;


// =====================================================
// USB GAME CONTROLLER
// =====================================================
//
// Dedicated HID axes:
//
// Accelerator = Hall sensor
// Brake       = Load cell
//

Joystick_ Joystick(
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_MULTI_AXIS,

  0,      // Buttons
  0,      // Hat switches

  false,  // X
  false,  // Y
  false,  // Z

  false,  // Rx
  false,  // Ry
  false,  // Rz

  false,  // Rudder
  false,  // Generic throttle

  true,   // Accelerator
  true,   // Brake

  false   // Steering
);


// =====================================================
// SETUP
// =====================================================

void setup() {

  // ---------------------------------------------------
  // Serial monitor
  // ---------------------------------------------------

  Serial.begin(115200);


  // ---------------------------------------------------
  // Throttle
  // ---------------------------------------------------

  pinMode(THROTTLE_PIN, INPUT);


  // ---------------------------------------------------
  // HX711
  // ---------------------------------------------------

  brakeScale.begin(HX711_DOUT, HX711_SCK);

  delay(500);


  // ---------------------------------------------------
  // Tare brake
  // ---------------------------------------------------
  //
  // KEEP YOUR FOOT OFF THE BRAKE WHILE POWERING ON.
  //

  if (brakeScale.is_ready()) {

    brakeScale.tare(15);

    Serial.println("HX711 ready - brake tared.");

  } else {

    Serial.println("WARNING: HX711 not ready.");
  }


  // ---------------------------------------------------
  // USB HID controller
  // ---------------------------------------------------

  Joystick.setAcceleratorRange(0, 1023);
  Joystick.setBrakeRange(0, 1023);

  Joystick.begin(false);


  Serial.println();
  Serial.println("PEDAL CONTROLLER STARTED");
  Serial.println();
}


// =====================================================
// MAIN LOOP
// =====================================================

void loop() {

  // ===================================================
  // THROTTLE
  // ===================================================

  int throttleRaw = analogRead(THROTTLE_PIN);


  // Endpoint deadzones
  int throttleStart =
      THROTTLE_RELEASED + THROTTLE_DEADZONE;

  int throttleEnd =
      THROTTLE_PRESSED - THROTTLE_DEADZONE;


  // Convert raw Hall value into HID range
  long throttleOutput =
      map(
        throttleRaw,
        throttleStart,
        throttleEnd,
        0,
        1023
      );


  throttleOutput =
      constrain(
        throttleOutput,
        0,
        1023
      );


  // ===================================================
  // BRAKE
  // ===================================================

  static long brakeOutput = 0;

  // Used for debug display
  static long brakeRaw = 0;


  // Only read HX711 when a new measurement exists.
  //
  // This avoids blocking the entire Arduino loop.

  if (brakeScale.is_ready()) {

    // ONE load-cell conversion
    brakeRaw = brakeScale.get_value(1);


    // -------------------------------------------------
    // FILTER
    // -------------------------------------------------
    //
    // 80% newest measurement
    // 20% previous measurement
    //

    if (!brakeFilterInitialized) {

      filteredBrake = brakeRaw;

      brakeFilterInitialized = true;

    } else {

      filteredBrake =
          (
            filteredBrake +
            (brakeRaw * 4L)
          ) / 5L;
    }


    // -------------------------------------------------
    // BRAKE MAPPING
    // -------------------------------------------------

    if (filteredBrake > BRAKE_START) {

      // Near zero force
      brakeOutput = 0;

    } else {

      brakeOutput =
          map(
            filteredBrake,
            BRAKE_START,
            BRAKE_FULL,
            0,
            1023
          );


      brakeOutput =
          constrain(
            brakeOutput,
            0,
            1023
          );
    }
  }


  // ===================================================
  // SEND VALUES TO WINDOWS
  // ===================================================

  Joystick.setAccelerator(throttleOutput);

  Joystick.setBrake(brakeOutput);

  Joystick.sendState();


  // ===================================================
  // DEBUG MONITOR
  // ===================================================

  if (
    millis() - lastDebugPrint
    >= DEBUG_INTERVAL
  ) {

    lastDebugPrint = millis();


    // ---------------- THROTTLE ----------------

    Serial.print("Throttle RAW: ");
    Serial.print(throttleRaw);

    Serial.print("   HID: ");
    Serial.print(throttleOutput);


    // ---------------- BRAKE ----------------

    Serial.print("      Brake RAW: ");
    Serial.print(brakeRaw);

    Serial.print("   Filtered: ");
    Serial.print(filteredBrake);

    Serial.print("   HID: ");
    Serial.println(brakeOutput);
  }


  // Fast USB/controller loop
  delay(1);
}