# Hardware Pin Mapping

This document describes the connections between the Arduino UNO and the hardware components used in the system.

| Component | Arduino Pin | Description |
|---|---|---|
| IR Receiver | D2 | Receives infrared remote commands |
| DHT11 Sensor | D3 | Temperature and humidity monitoring |
| HC-SR04 Trig | D4 | Ultrasonic trigger signal |
| HC-SR04 Echo | D5 | Ultrasonic echo signal |
| Servo Motor | D6 | Door locking mechanism |
| Buzzer | D7 | Audio alerts |
| Mode Button | D8 | Switches operating modes |
| MAX7219 DIN | D9 | Matrix data input |
| MAX7219 CLK | D10 | Matrix clock |
| MAX7219 CS | D11 | Matrix chip select |
| Room LED | D12 | Room lighting indicator |
| Alarm LED | D13 | Security alarm indicator |
| LDR Sensor | A5 | Ambient light detection |

## Notes

- Four MAX7219 8×8 LED matrix modules are connected in series.
- The LED matrices form a combined 32×8 display.
- The Raspberry Pi 4 Model B communicates with the Arduino through serial communication.
