# Project Specification

> **Smart Temperature Control and Monitoring System**
>
> A dual-ATmega64 embedded system for intelligent temperature monitoring,
> automatic fan control, and user interaction through UART communication.

---

## 📚 Table of Contents

- [Project Overview](#project-overview)
- [Objectives](#objectives)
- [Hardware Components](#hardware-components)
- [Functional Requirements](#functional-requirements)
- [Operating Modes](#operating-modes)
- [Communication Protocol](#communication-protocol)
- [Timing Requirements](#timing-and-scheduling-requirements)
- [Display Unit Behavior](#display-unit-behavior)

---

# Project Overview

The objective of this project is to design and simulate a **Smart Temperature
Control and Monitoring System** utilizing **two ATmega64 microcontrollers**
communicating through **USART**.

The system continuously measures ambient temperature, dynamically controls a
DC cooling fan, provides a user interface using a LCD and physical
buttons, and generates both visual and audible alerts whenever the system
reaches critical thermal conditions.

---

# Objectives

- Develop a dual-node embedded system using C
- Utilize hardware peripherals (ADC, PWM, Timers, USART)
- Implement reliable UART communication
- Design a completely non-blocking scheduler
- Simulate the complete hardware in Proteus

---

# Hardware Components

| Component | Description |
|-----------|-------------|
| **MCU 1** | ATmega64 @ 8 MHz (Control Unit) |
| **MCU 2** | ATmega64 @ 8 MHz (Display Unit) |
| **Temperature Sensor** | LM35 |
| **Motor Driver** | L298 H-Bridge |
| **Cooling Device** | DC Motor (Fan) |
| **Display** | LM032L 20×2 LCD |
| **Indicators** | Green, Yellow, Red LEDs |
| **Alarm** | Active Buzzer (5V) |
| **User Inputs** | Mode, Up, Down Push Buttons |

---

# Functional Requirements

## 🌡 Temperature Control

The Control Unit shall automatically adjust the cooling fan speed using
**hardware PWM** according to the measured ambient temperature.

| Temperature Range | System State | Fan Speed |
|:-----------------|:------------:|----------:|
| < 25°C | NORMAL | 0% |
| 25–29.9°C | LOW | 30% |
| 30–34.9°C | MEDIUM | 50% |
| 35–39.9°C | HIGH | 75% |
| ≥ 40°C | CRITICAL | 100% |

---

## Hysteresis

To prevent rapid oscillation near threshold temperatures, the system shall
implement **1°C hysteresis**.

**Example**

```
29.8°C → LOW

30.0°C → MEDIUM

29.6°C → remains MEDIUM

28.9°C → LOW
```

---

# Operating Modes

## Auto Mode

- Temperature determines fan speed.
- Hysteresis logic is active.
- User cannot directly control the fan.


## Manual Mode

The user manually selects one of the following speeds:

- 0%
- 25%
- 50%
- 75%
- 100%

Although manual speed overrides the PWM output, the temperature monitoring
logic continues executing in the background.

> LEDs and system state always reflect the measured temperature, even while
> operating in Manual Mode.

---

# Communication Protocol

## UART Configuration

| Parameter | Value |
|-----------|-------|
| Baud Rate | 9600 bps |
| Data Bits | 8 |
| Stop Bits | 1 |
| Parity | None |


## Control Unit → Display Unit

Every **1000 ms**, transmit

```text
T:[Temp],F:[Speed],S:[STATE]
```

Example

```text
T:32,F:50,S:MEDIUM
```


## Display Unit → Control Unit

| Command | Meaning |
|----------|---------|
| `A` | Auto Mode |
| `M:25` | Manual Fan = 25% |
| `M:50` | Manual Fan = 50% |
| `M:75` | Manual Fan = 75% |
| `M:100` | Manual Fan = 100% |

---

# Timing and Scheduling Requirements

| Task | Period |
|------|-------:|
| ADC Sampling | 100 ms |
| UART Transmission | 1000 ms |
| Button Debouncing | 50 ms |
| Buzzer Toggle | 500 ms |
| UART Timeout | 3000 ms |

> Long blocking delays such as `delay_ms()` shall **not** be used for system
> scheduling.

---

# Display Unit Behavior

## LCD

Displays:

```
TEMP: 28 C AUTO
FAN : 30% LOW
```

---

## LED Indicators

| LED | Condition |
|-----|-----------|
| 🟢 Green | NORMAL |
| 🟡 Yellow | LOW / MEDIUM / HIGH |
| 🔴 Red | CRITICAL |


## Buzzer

The buzzer shall

- activate only in **CRITICAL** state
- toggle every **500 ms**


## UART Error Handling

If no valid UART packet is received within **3000 ms**,

the Display Unit shall

- Clear the LCD
- Display

```
UART ERROR
```

- Turn off all LEDs
- Disable the buzzer

---

# ✅ Summary

This project combines multiple embedded-system peripherals—including **ADC,
PWM, USART, Timers, GPIO, LCD, and Interrupts**—to implement a fully
non-blocking smart temperature control system using two communicating
ATmega64 microcontrollers.