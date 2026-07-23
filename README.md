# Smart Temperature Control System

> A dual-ATmega64 embedded system for intelligent temperature monitoring, automatic fan speed control, and bidirectional UART communication.

---

## 📚 Table of Contents

- [Overview](#overview)
- [System Architecture](#system-architecture)
- [Features](#features)
- [Repository Structure](#repository-structure)
- [Documentation](#documentation)
- [Hardware](#hardware)
- [Technologies](#technologies)
- [Running the Simulation](#running-the-simulation)
- [Building the Firmware](#building-the-firmware)

---

# Overview

This project implements a **Smart Temperature Control and Monitoring System**
using two **ATmega64 microcontrollers** that communicate with each other through
UART communication.

The system measures the surrounding temperature using an **LM35 temperature
sensor** and automatically adjusts the cooling fan speed based on the detected
temperature. It also provides a user interface using an LCD display, status
LEDs, a buzzer alarm, and push buttons for user interaction.

The firmware is designed using an **interrupt-driven architecture**, allowing
the system to respond quickly to temperature changes, user inputs, and
communication events without relying on blocking delays.

---

# System Architecture

The system consists of two ATmega64 microcontrollers:

- **Control Unit**
  - Reads temperature from the LM35 sensor using ADC
  - Calculates required fan speed
  - Generates PWM signal for motor control
  - Communicates system status through UART

- **Display Unit**
  - Displays temperature and system status on LCD
  - Handles user input buttons
  - Controls LEDs and buzzer alarms
  - Sends user commands to the Control Unit through UART

--- 

# Features

- Dual-microcontroller architecture
- Hardware PWM fan control
- Manual fan speed override
- Bidirectional UART communication
- UART timeout detection
- Interrupt-driven scheduling
- Proteus simulation included

---

# Repository Structure

```text
.
├── control-unit/
├── display-unit/
├── simulation/
├── docs/
│   ├── project_specification.md
│   └── implementation-report.md
└── README.md
```

Each firmware directory follows the same layout:

```text
control-unit/
│
├── include/           Header files (.h)
├── src/               Source files (.c)
└── out/               Pre-generated firmware (.hex)
```
---

# Documentation

This README provides a high-level overview of the project.

For detailed information about the system design, hardware configuration and implementation details refer to the documentation files below.

| Document | Description |
|----------|-------------|
| [**Project Specification**](docs/project_specification.md) | Functional requirements, hardware specification, communication protocol, and expected system behavior. |
| [**Implementation Report**](docs/implementation_report.md) | Firmware architecture, implementation details, engineering decisions, and module descriptions. |

---

# Hardware

| Component | Description |
|-----------|-------------|
| MCU 1 | ATmega64 (Control Unit) |
| MCU 2 | ATmega64 (Display Unit) |
| Temperature Sensor | LM35 |
| Motor Driver | L298 H-Bridge |
| Cooling Device | DC Motor |
| Display | 20×2 LCD |
| Indicators | Green, Yellow, Red LEDs |
| Alarm | Active Buzzer |
| User Input | Mode, Up, Down Buttons |

---

# Technologies

- C
- ATmega64
- ADC
- Hardware PWM
- USART
- Timer Interrupts
- GPIO
- Proteus Design Suite

---

# Running the Simulation

Pre-generated firmware files are already included.

Simply:

1. Open the Proteus project inside `simulation/`.
2. Load the firmware from:
   - `control-unit/out/`
   - `display-unit/out/`
3. Start the simulation.
4. Adjust the LM35 temperature to observe the system behavior.

> You do **not** need to compile the firmware to run the simulation.

---

# Building the Firmware

This repository is not bound to a specific IDE or compiler.
You may use any AVR-compatible development environment, including:

- CodeVisionAVR
- Microchip Studio (Atmel Studio)
- AVR-GCC
- or any other AVR toolchain

To build the firmware:

1. Create a new **ATmega64** project.
2. Configure the MCU clock to **8 MHz**.
3. Add every `.c` file from the corresponding `src/` directory.
4. Add the `include/` directory to the compiler include paths.
5. Build the project.

> Every IDE manages projects differently. Since this repository is
> IDE-independent, configuring the project structure is left to the user.

---
