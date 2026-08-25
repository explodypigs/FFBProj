# FFB Racing Wheel Controls

Arduino firmware for the input side of a DIY force-feedback racing wheel project. The sketches expose the wheel's paddle shifters and pedals to Windows as USB game-controller inputs.

## Project layout

| Sketch | Purpose |
| --- | --- |
| `paddleShifters/paddleShifters.ino` | Presents the two shift paddles as USB buttons. |
| `Pedals/calibration/calibration.ino` | Prints raw throttle and brake readings so the pedal endpoints can be measured. |
| `Pedals/FullJoysticks/FullJoysticks.ino` | Runs the finished pedal controller and presents throttle and brake as dedicated USB axes. |

## Hardware and libraries

The USB controller sketches require an Arduino with native USB HID support, such as a Leonardo, Micro, or compatible ATmega32U4 board.

- [Arduino Joystick Library](https://github.com/MHeironimus/ArduinoJoystickLibrary) for Windows game-controller output
- [HX711 Arduino Library](https://github.com/bogde/HX711) for the brake load-cell amplifier
- 49E Hall-effect sensor for throttle position
- Load cell and HX711 amplifier for brake force
- Two normally-open paddle switches

## Wiring

### Paddle shifters

| Control | Arduino pin | Connection behavior |
| --- | --- | --- |
| Downshift | D4 | Switch connects the pin to ground when pressed |
| Upshift | D2 | Switch connects the pin to ground when pressed |

The sketch enables the Arduino's internal pull-up resistors, so no external pull-up resistors are required. A pressed switch therefore reads `LOW`.

### Pedals

| Device | Signal | Arduino pin |
| --- | --- | --- |
| Throttle Hall sensor | Analog output | A2 |
| HX711 | DT / DOUT | D2 |
| HX711 | SCK / CLK | D4 |

Power and ground must match the requirements of the selected sensors and board. All modules should share a common ground.

## How the firmware works

### Paddle shifters

The paddle sketch creates a two-button USB gamepad. Each switch uses a 10 ms software debounce: a change is only reported after the electrical reading has remained stable. Downshift appears as Windows button 1 and upshift as Windows button 2.

### Pedal calibration

The calibration sketch tares the brake load cell at startup, then prints the raw Hall-sensor and load-cell readings at 115200 baud. Keep the brake released while powering on. Record the throttle values at rest and full travel, and record the brake reading at the desired maximum force.

Enter those measurements in `Pedals/FullJoysticks/FullJoysticks.ino`:

- `THROTTLE_RELEASED`: raw throttle value with the pedal released
- `THROTTLE_PRESSED`: raw throttle value at full travel
- `BRAKE_START`: threshold below which brake force begins to register
- `BRAKE_FULL`: raw reading at the desired maximum braking force

The current brake values are negative because of the installed load-cell orientation. If your readings increase under load, use calibration endpoints that match that direction and adjust the start comparison/mapping accordingly.

### Full pedal controller

At startup, the pedal sketch tares the load cell and creates a USB multi-axis controller with dedicated accelerator and brake axes. The throttle reading is mapped between its calibrated endpoints, with a small endpoint deadzone, into the HID range `0-1023`.

The brake is sampled only when the HX711 has a new conversion, avoiding a blocking controller loop. A lightweight smoothing filter uses 80% of the newest sample and 20% of the previous filtered result, then maps force into the same `0-1023` HID range. Both axes are sent together in one USB report. Diagnostic values are printed every 100 ms without slowing the main loop substantially.

## Setup and upload

1. Install the Joystick and HX711 libraries in the Arduino IDE.
2. Connect the pedal hardware and upload `Pedals/calibration/calibration.ino`.
3. Open Serial Monitor at 115200 baud, measure the endpoints, and update the calibration constants in the full pedal sketch.
4. Upload `Pedals/FullJoysticks/FullJoysticks.ino` to the pedal controller.
5. Upload `paddleShifters/paddleShifters.ino` to the paddle controller.
6. In Windows, open **Set up USB game controllers** to verify the two buttons and both pedal axes, then bind them in the racing simulator.

> Keep the brake completely released whenever the pedal controller powers on, because startup establishes the zero-force reference.

## Relationship to force feedback

These sketches handle driver inputs only. Steering position, motor control, force-feedback effects, power electronics, and safety limits belong to the wheel-base controller and are intentionally outside this folder.
