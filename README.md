# Smart-Home-Automation-and-Security-System
Smart home system combining environmental monitoring, automated response to monitored changes, security alerts for emergency cases, remote control, and a mini game in LED matrix display. The system has 3 modes; 
1. Auto Mode - Fully Automated response to environmental changes or emergency cases.
2. Manual Mode- IR remote controller-based control over the whole system
3. Game Mode - Entertainment focused mode inside the Manual Mode, containing a dino game. 

## Project Overview

This project aims to integrate multiple electronic components into a single smart home system. It uses an Arduino microcontroller to interact with sensors, a servo motor, an infrared remote controller, and a four-module LED matrix display.

A Raspberry Pi is also part of the hardware setup for potential advanced processing and computer vision functionality for security system.

## Features

- **Dual Operating Modes:** Autonomous and manual control.
- **Environmental Monitoring:** Temperature and humidity measurement using a DHT11 sensor.
- **Automatic Lighting:** Light-level detection using an LDR sensor.
- **Proximity Detection:** Distance measurement using an HC-SR04 ultrasonic sensor.
- **Servo-Based Door Lock:** Servo motor control for a door-lock prototype.
- **Security Alerts:** Buzzer and LED indicators for visual and audible alerts.
- **Infrared Remote Control:** Remote operation of supported system functions.
- **LED Matrix Display:** A 32×8 display built from four chained 8×8 LED matrices.
- **Dino Game Mode:** An interactive mi     ni-game designed for the LED matrix.

*Note: The final implementation and testing status of each feature will be documented as the project is validated.*

## Hardware Components

The main components include:

- Arduino UNO
- Raspberry Pi 4
- DHT11 temperature and humidity sensor
- HC-SR04 ultrasonic distance sensor
- Light-dependent resistor (LDR)
- SG90 servo motor
- Infrared receiver and remote controller
- Four 8×8 LED matrix modules with MAX7219 control
- Buzzer
- LEDs
- Push button
- Breadboard and jumper wires

## System Architecture

The Arduino serves as the main controller for the connected sensors and actuators. Sensor readings and control inputs are used to determine the system's responses.

The Raspberry Pi provides a platform for additional processing tasks that may be integrated into the project.

The LED matrix displays information and provides the interface for the Dino mini-game.

A detailed system architecture diagram and circuit schematic will be added as the hardware documentation is completed.

## Repository Structure

```text
smart-home-automation-and-security-system/
├── README.md
├── arduino/
├── hardware/
├── docs/
├── media/
└── tests/
```

The directories will contain the source code, hardware documentation, project images, usage instructions, and test results as they are prepared.

## Getting Started

To reproduce the project, you will need the listed hardware components, a compatible Arduino development environment, and the required libraries.

Detailed wiring instructions, library dependencies, installation steps, and operating instructions will be provided in the corresponding documentation.

## Testing and Results

Testing documentation, demonstration images, and video links will be added as the system's functions are verified.

## Future Improvements

Potential improvements include:

- Integrating and validating Raspberry Pi-based computer vision.
- Completing the circuit schematic and wiring documentation.
- Expanding system testing and reliability checks.
- Improving documentation and the user interface.

## License

A license will be added after the project's licensing terms have been selected.
