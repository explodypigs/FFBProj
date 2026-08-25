#include <HX711.h>

// -----------------------
// PIN DEFINITIONS
// -----------------------
const int THROTTLE_PIN = A2;

const int HX711_DOUT = 2;   // DT -> D2
const int HX711_SCK  = 4;   // SCK -> D4

HX711 brakeScale;

void setup() {
  Serial.begin(115200);

  brakeScale.begin(HX711_DOUT, HX711_SCK);

  Serial.println("Pedal calibration test");
  Serial.println("DO NOT press the brake while starting.");

  delay(1500);

  if (brakeScale.is_ready()) {
    brakeScale.tare(20);
    Serial.println("HX711 ready and zeroed.");
  } else {
    Serial.println("ERROR: HX711 not detected.");
  }
}

void loop() {
  int throttleRaw = analogRead(THROTTLE_PIN);

  Serial.print("Throttle: ");
  Serial.print(throttleRaw);

  Serial.print("    Brake: ");

  if (brakeScale.is_ready()) {
    long brakeRaw = brakeScale.get_value(3);
    Serial.println(brakeRaw);
  } else {
    Serial.println("HX711 NOT READY");
  }

  delay(100);
}