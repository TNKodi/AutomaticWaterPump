# Automatic Water Pump Controller

A collaborative project  that automates water-level monitoring and pump control using a wireless transmitter–receiver pair. The transmitter measures tank water level via an ultrasonic sensor, filters the reading on an MCU, and sends it over an nRF300 RF module. The receiver decodes the level, lights up one of five LEDs to indicate the approximate water level, and toggles a relay to turn the pump on or off.

---

## Table of Contents

1. [Overview](#overview)  
2. [Features](#features)  
3. [System Architecture](#system-architecture)  
4. [Hardware Requirements](#hardware-requirements)  
   - [Transmitter Unit](#transmitter-unit)  
   - [Receiver Unit](#receiver-unit)  
5. [Software Requirements](#software-requirements)  
6. [Wiring Diagrams](#wiring-diagrams)  
   - [Transmitter Schematic](#transmitter-schematic)  
   - [Receiver Schematic](#receiver-schematic)  
7. [Installation & Setup](#installation--setup)  
   - [Flashing Transmitter Firmware](#flashing-transmitter-firmware)  
   - [Flashing Receiver Firmware](#flashing-receiver-firmware)  
8. [Configuration](#configuration)  
9. [Usage](#usage)  
10. [Troubleshooting](#troubleshooting)  
11. [Contributing](#contributing)  
12. [License](#license)  

---

## Overview

This project implements an automated water-pump controller that consists of two separate PCB/modules:

- **Transmitter Unit** (installed near or inside the water tank):  
  - Measures water depth with an ultrasonic sensor (HC-SR04).  
  - Applies a simple moving-average filter on the MCU to smooth out spurious readings.  
  - Packages the filtered water-level data and transmits it wirelessly using an nRF300 RF module.

- **Receiver Unit** (installed near the pump):  
  - Receives wireless packets from the transmitter via its paired nRF300 module.  
  - Decodes the water-level value and illuminates one of five LEDs (representing 0–20%, 21–40%, 41–60%, 61–80%, and 81–100% of tank capacity).  
  - Activates a relay to switch the water pump ON when the water level falls below a user-defined threshold (e.g., < 20%), and turns the pump OFF once the level exceeds another threshold (e.g., > 80%).

By separating sensing and actuation into two modules, the system avoids running power cables or sensor wires down the tank to the pump. Instead, only a single short-range RF link (nRF300, 433/868 MHz) carries the water-level information.

---

## Features

- **Wireless Water-Level Monitoring**  
  - Ultrasonic sensing at the transmitter, with on-MCU filtering to reduce noise.  
  - nRF300 RF module for reliable transmission up to ~50–100 meters (line-of-sight).

- **Five-Level LED Indicator**  
  - Receiver lights up a bank of five LEDs indicating approximate percentage-full of the tank.

- **Automated Pump Control**  
  - Relay‐driven pump ON/OFF based on configurable “low‐level” and “high‐level” thresholds.  
  - Hysteresis prevents rapid on/off switching (e.g., pump turns OFF once level > 80%, turns ON once level < 20%).

- **Low Power Consumption**  
  - Transmitter MCU sleeps between measurements to conserve battery (optional, see configuration).  
  - Receiver MCU only wakes briefly on valid RF packets.

- **Modular Design**  
  - Separate transmitter and receiver codebases (e.g., `transmitter.ino` and `receiver.ino`).  
  - Supports easy replacement of ultrasonic sensor, RF module, or microcontroller.

---

## System Architecture

```text
┌────────────────────────┐      ┌────────────────────────┐      ┌───────────────┐
│    Transmitter Unit    │      │    Wireless Link       │      │  Receiver Unit│
│  (e.g., Arduino Nano)  │──────▶ nRF300 (TX @ 433/868MHz) ───▶│  (e.g., Arduino UNO) │
│                        │      │    nRF300 (RX)         │      │               │
│  ┌──────────────────┐  │      └────────────────────────┘      │               │
│  │ Ultrasonic Sensor│  │                                      │ ┌───────────┐ │
│  │    (HC-SR04)     │  │                                      │ │LED Bank   │ │
│  └──────────────────┘  │                                      │ │(5× LEDs)  │ │
│   (Filters readings   │                                      │ └───────────┘ │
│    via moving-average)│                                      │     │         │
│                       │                                      │ ┌───────────┐ │
│  ┌──────────────────┐ │                                      │ │ Relay     │ │
│  │      MCU         │ │                                      │ │ Module    │ │
│  │ (ATmega328P, etc.)│                                      │ └───────────┘ │
│  └──────────────────┘ │                                      └────────────────┘
└────────────────────────┘
````

## Hardware Requirements

### Transmitter Unit

- **Microcontroller: ATtiny84** (8-bit AVR, 14-pin package)  
- **Ultrasonic Sensor (HC-SR04)** for water-level measurement  
- **nRF300 RF Transceiver Module** (433 MHz or 868 MHz variant)  
- **3.3 V Voltage Regulator** (e.g., an LD1117A or MIC5205) to power the nRF300 from a 5 V supply  
- **Crystal or Resonator (Optional)** for precise timing on the ATtiny84 (if you’re not using the internal oscillator)  
- **Breadboard or Custom PCB** for prototyping  
- **Jumper Wires** (Dupont cables)  
- **Power Source**:  
  - 5 V DC (e.g., USB-to-5 V adapter, battery pack, or DC-DC converter) for the ATtiny84 and sensor  
  - If battery-powered, a low-dropout regulator (LDO) to step down to 5 V and 3.3 V  

### Receiver Unit

- **Microcontroller: ATtiny84** (8-bit AVR, 14-pin package)  
- **nRF300 RF Transceiver Module** (matching frequency of the transmitter)  
- **Relay Module (1-channel)** (5 V coil to switch the pump power)  
- **5 LEDs** (e.g., green, yellow, red) for water-level indication  
  - **Current-limiting resistors** (220 Ω – 330 Ω) for each LED  
- **Water Pump** (e.g., 12 V DC submersible or AC pump, matched to your relay contact rating)  
- **Flyback Diode** (e.g., 1N4007) across the relay coil, if not built-in  
- **Power Supply**:  
  - 5 V for the ATtiny84 (regulated)  
  - 12 V DC (or AC mains) for the pump (wired through the relay)  
- **Breadboard or Custom PCB** for prototyping  

> **Note:** Each ATtiny84’s GND pin must share a common ground with its nRF300 module. The transmitter and receiver do not share grounds with each other—only locally on each board.

---

## Software Requirements

- **Arduino IDE** (v1.8.13 or later) with support for **ATtiny cores** (e.g., “ATtinyCore” by Spence Konde) or **AVR GCC toolchain** in PlatformIO  
- **TinyWireM** (if using I²C peripherals or EEPROM libraries) _or_ **NewPing** (for HC-SR04 timing)  
- **RF24 Library** compatible with nRF300 (e.g., “RF24” by TMRh20, configured to use USI or SPI on ATtiny84)  
- **AVRDUDE** (for manual flashing, if not using Arduino IDE)  
- **Git** (to clone the repository, optional if downloading ZIP)  

---

## Wiring Diagrams

Below are block diagrams and pin assignments. Adjust pin numbers in code if your wiring differs. All ATtiny84 pin references use the physical package names (e.g., PA0, PA1, PB2). Refer to your ATtiny84-pinout diagram when wiring.

### Transmitter Schematic


