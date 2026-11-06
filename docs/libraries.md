# Libraries and Dependencies

The Arduino firmware uses the following libraries:

| Library | Header File | Purpose |
|---|---|---|
| DHT Sensor Library | `DHT.h` | Temperature and humidity monitoring |
| Servo | `Servo.h` | Servo motor control |
| Arduino-IRremote | `IRremote.hpp` | Infrared remote control |
| MD_MAX72XX | `MD_MAX72xx.h` | MAX7219 LED matrix control |

## Installation

The required libraries can be installed through the Arduino IDE Library Manager.

1. Open Arduino IDE.
2. Navigate to Sketch → Include Library → Manage Libraries.
3. Search for each library listed above.
4. Install the required libraries.

## Hardware Platform

- Microcontroller: Arduino UNO
- Additional computing platform: Raspberry Pi 4 Model B
- Display: Four MAX7219 8×8 LED matrix modules
