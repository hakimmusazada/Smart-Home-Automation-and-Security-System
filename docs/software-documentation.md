# Software Documentation

## 1. System Overview

The Smart Home Automation and Security System is an Arduino-based embedded system designed to provide environmental monitoring, automated home control, and security functionality.

The system integrates multiple sensors, actuators, and a Raspberry Pi 4 Model B to perform different tasks. Users can interact with the system through an infrared remote control, while automatic operations are handled by the Arduino.

The system also includes a Dino mini-game displayed on a 32×8 LED matrix.

## 2. Operating Modes

The system provides three main operating modes: Automatic Mode, Manual Mode, and Game Mode.

### 2.1 Automatic Mode

In Automatic Mode, the Arduino monitors environmental conditions and performs predefined actions based on sensor readings.

- The LDR sensor monitors ambient light levels and controls the room lighting.
- The HC-SR04 ultrasonic sensor detects nearby objects or presence.
- When a security-related detection occurs, the system initiates a face verification process through the Raspberry Pi.
- The system responds according to the verification result.

### 2.2 Manual Mode

Manual Mode allows users to control system functions using an infrared remote control.

The available commands include:

- Displaying temperature and humidity.
- Displaying the ambient light level.
- Turning the room light ON or OFF.
- Locking and unlocking the door.
- Entering and exiting Game Mode.

### 2.3 Game Mode

Game Mode features a Dino-style mini-game displayed on four chained MAX7219 8×8 LED matrices, forming a 32×8 display.

The player controls the dinosaur using the infrared remote control. The objective is to jump over obstacles and achieve the highest possible score.

The game includes obstacle generation, collision detection, score tracking, and increasing game speed.

## 3. Sensor Monitoring

The system uses several sensors to monitor environmental conditions and detect nearby objects.

- **DHT11:** Measures temperature and humidity.
- **LDR:** Detects ambient light intensity.
- **HC-SR04:** Measures distance using ultrasonic waves.

The Arduino reads sensor data and uses it to determine the appropriate system responses.

## 4. Security System

The security subsystem combines ultrasonic detection with face verification.

When the ultrasonic sensor detects a nearby object or presence in Automatic Mode, the Arduino initiates a verification request to the Raspberry Pi.

The system supports two possible verification outcomes:

- **Recognized face:** The door servo mechanism is activated to unlock the door temporarily, and a welcome message is displayed.
- **Unrecognized face:** The alarm is activated through the buzzer and alarm LED, and an alert message is displayed.

The security process also includes a timeout mechanism if no verification response is received.

## 5. Remote Control

An infrared receiver is used to receive commands from the remote control.

In Manual Mode, different remote buttons are assigned to specific functions, including sensor data display, lighting control, door locking and unlocking, and Game Mode operation.

The remote control also provides dedicated commands for jumping and restarting the Dino game.

## 6. Raspberry Pi Communication

The Raspberry Pi 4 Model B communicates with the Arduino through serial communication.

The Raspberry Pi is responsible for the face recognition process, while the Arduino handles the physical responses.

The Arduino recognizes the following serial commands:

- `CMD:FACE_KNOWN` — Indicates successful face recognition.
- `CMD:FACE_UNKNOWN` — Indicates an unrecognized face.
- `CMD:PING` — Communication test command.

Based on the received commands, the Arduino controls the servo, buzzer, LEDs, and display.

## 7. Dino Game

The Dino game is implemented using the MD_MAX72xx library and four MAX7219 LED matrix modules.

Its main features include:

- Dinosaur character animation.
- Random cactus obstacle generation.
- Jump control through the infrared remote.
- Collision detection.
- Score tracking.
- Increasing difficulty through game speed adjustment.
- Game-over animation and restart functionality.
