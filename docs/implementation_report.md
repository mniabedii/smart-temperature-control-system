# ⚙️ Implementation Report  
## Smart Temperature Control System

> This document describes the hardware implementation, firmware architecture,
> and engineering decisions behind the dual-ATmega64 Smart Temperature Control
> System.

---

## 📚 Table of Contents

- [1. Overall System Architecture](#1-overall-system-architecture)

- [2. Hardware Implementation](#2-hardware-implementation)
  - [2.1 Control Unit Hardware (MCU1)](#21-control-unit-hardware-mcu1)
  - [2.2 Display Unit Hardware (MCU2)](#22-display-unit-hardware-mcu2)

- [3. Software Implementation](#3-software-implementation)
  - [3.1 Non-Blocking Task Scheduler](#31-non-blocking-task-scheduler-timer-0)
  - [3.2 Control Unit Firmware (MCU1)](#32-control-unit-firmware-mcu1)
  - [3.3 Display Unit Firmware (MCU2)](#33-display-unit-firmware-mcu2)

---

# 1. Overall System Architecture

The project utilizes a distributed, two-node architecture to simulate a
standard industrial control system.

---

## 🖥️ Microcontroller 1 — Control Unit (MCU1)

Acts as the **Remote Terminal Unit**.

Responsibilities:

- Interfaces directly with the analog physical world.
- Handles temperature sensing.
- Controls motor actuation.
- Generates fan speed commands.

---

## 🖥️ Microcontroller 2 — Display Unit (MCU2)

Acts as the **Human-Machine Interface**.

Responsibilities:

- Handles user input.
- Parses received telemetry.
- Drives the LCD display.
- Manages visual and auditory alarms.

---

## 🔗 Communication Bridge

The two nodes communicate through a:

| Parameter | Value |
|-----------|-------|
| Protocol | Asynchronous USART |
| Baud Rate | 9600 bps |
| Direction | Bidirectional |

The UART link allows:

- MCU1 → MCU2:
  - Temperature telemetry
  - System status information

- MCU2 → MCU1:
  - Manual override commands
  - User control requests

---

# 2. Hardware Implementation

The hardware design carefully isolates low-power logic components from
high-power actuators and noisy analog inputs.

---

# 2.1 Control Unit Hardware

## Microcontroller

**ATmega64**

Configuration:

| Parameter | Value |
|-----------|-------|
| Clock Source | Internal RC Oscillator |
| Frequency | 8.000 MHz |

---

## 🌡️ Analog-to-Digital Conversion (LM35)

The LM35 temperature sensor provides a linear voltage output:

```
10mV = 1°C
```

Connection:

| Component | MCU Pin |
|-----------|---------|
| LM35 Output | PF0 (ADC0) |

---

## Reference Voltage Design

To ensure accurate ADC readings:

- AVCC is connected directly to the +5V logic rail.
- AREF is decoupled to ground using a 100nF capacitor.

This filters high-frequency digital noise and prevents ADC measurement
fluctuations.

---

## ⚙️ Motor Driver (L298 H-Bridge)

Microcontroller GPIO pins cannot directly drive a DC motor because they can only
source approximately 20mA.

The L298 provides:

- Current amplification
- Voltage isolation
- Motor driving capability

---

## Direction Control

Connections:

| MCU Pin | L298 Pin |
|---------|----------|
| PB0 | IN1 |
| PB1 | IN2 |

Motor direction:

```
PB0 = HIGH
PB1 = LOW

→ Forward Rotation
```

---

## Speed Control (PWM)

The PWM signal is generated using:

```
PB5 → OC1A → L298 ENA
```

Hardware Timer 1 PWM directly controls the motor speed.

Changing the duty cycle changes the average motor voltage:

```
Higher Duty Cycle → Higher Speed
Lower Duty Cycle → Lower Speed
```

---

# 2.2 Display Unit Hardware (MCU2)

## 📟 LCD Display (LM016L)

A standard 20x2 LCD is used.

Configuration:

- 4-bit communication mode
- Connected to PORTA
- Driven using CodeVisionAVR `<alcd.h>` layout

LCD pins:

- RS
- RD
- EN
- D4-D7

---

## Indicator LEDs

Three LEDs indicate system status:

| LED | Pin |
|-|-|
| Green | PC0 |
| Yellow | PC1 |
| Red | PC2 |

---

## 🔊 Audible Alarm (Active Buzzer)

Connected to:

```
PC3
```

Because it is an active buzzer:

```
HIGH signal → Sound generation
```

No PWM signal is required.

---

## 🔘 User Input Buttons

Buttons:

| Button | Pin |
|-|-|
| Mode | PD0 |
| Up | PD1 |
| Down | PD2 |

The internal ATmega64 pull-up resistors are enabled.

Behavior:

```
Idle state:
Pin = HIGH

Button pressed:
Pin = LOW
```

This reduces external component requirements.

---

# 3. Software Implementation

The firmware is written in strict **C89 standard**.

The software architecture is:

- Modular
- Interrupt-driven
- Non-blocking
- RTOS-like scheduling approach

---

# 3.1 Non-Blocking Task Scheduler (Timer 0)

## Blocking Delay Removal

The project avoids:

```c
delay_ms()
```

because blocking delays:

- Freeze CPU execution.
- Cause missed UART messages.
- Reduce button responsiveness.

---

## Timer Configuration

Timer 0 operates in:

```
CTC (Clear Timer on Compare Match)
```

Configuration:

| Parameter | Value |
|-|-|
| CPU Clock | 8 MHz |
| Prescaler | 64 |
| Timer Frequency | 125 kHz |
| OCR0 | 124 |
| Interrupt Period | 1 ms |

---

The interrupt updates software timers:

```c
adc_timer++;
button_timer++;
uart_timer++;
```

The main loop executes tasks when their timers expire.

Example:

```
adc_timer >= 100

→ Read ADC
→ Reset timer
```

---

# 3.2 Control Unit Firmware (MCU1)


## ⚡ Hardware PWM Generation

Timer 1 is configured as:

```
8-bit Fast PWM
```

Configuration:

- WGM10
- WGM12
- COM1A1

PWM resolution:

```
0 - 255
```

Example:

For 75% speed:

```
PWM = (75 × 255) / 100

PWM = 191
```

Writing:

```c
OCR1A = 191;
```

updates the PWM duty cycle completely in hardware.

---

## ADC Processing

The ADC reads LM35 voltage.

To allow ADC stabilization after channel switching:

```asm
nop
nop
nop
nop
```

are executed.

The 10-bit ADC value is converted into Celsius temperature.

---

## Hysteresis Control Algorithm

To prevent fan oscillation near temperature boundaries, a 1°C hysteresis gap is used.

Example:

## Rising Temperature

```
30°C
↓
LOW → MEDIUM
```

## Falling Temperature

```
<29°C
↓
MEDIUM → LOW
```

---

## String Formatting in RAM

CodeVisionAVR requires strings passed to `sprintf()` to exist in SRAM.

Therefore:

```c
char current_state_str[15];
```

is created.

A switch statement copies the required state string into RAM before transmission.

---

# 3.3 Display Unit Firmware (MCU2)

## UART Message Parsing

The USART RX interrupt:

1. Receives characters.
2. Stores them in `rx_buffer`.
3. Detects newline termination.
4. Flags the main loop.

---

## Button Debouncing & Edge Detection

Buttons are checked every:

```
50 ms
```

The software compares:

- Current state
- Previous state

A command executes only when:

```
Current = LOW
Previous = HIGH
```

This detects only the button press event.

---

## 🔊 Asynchronous Buzzer Toggle

Critical temperature alarm:

```
500ms ON
500ms OFF
```

The main loop toggles the buzzer without blocking execution.

When leaving critical mode:

```c
buzzer_timer = 500;
```

ensures the alarm activates immediately next time.

---

## UART Watchdog Failsafe

A timeout counter increments every millisecond.

When valid UART data arrives:

```
uart_timeout = 0
```

If communication fails:

```
uart_timeout > 3000ms
```

The Display Unit:

- Stops motor operation.
- Clears LCD.
- Displays:

```
UART ERROR
```