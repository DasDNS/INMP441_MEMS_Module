# MEMS Microphone Experiments (INMP441 / ICS-43434)

This repository contains a collection of **experiments and example projects using MEMS microphones with I2S output**, primarily focused on the **INMP441** omnidirectional microphone module.  
All code in this repository is also **compatible with the ICS-43434** MEMS microphone, as both devices share a similar I2S digital audio interface.

---

## 🎯 Project Goals

- Learn and experiment with **digital MEMS microphones**
- Understand **I2S audio data capture**
- Explore audio sampling, buffering, and processing on microcontrollers
- Build reusable reference code for future audio-based projects

---

## 🎤 Supported Microphones

### INMP441 (Primary)
- Omnidirectional MEMS microphone
- Digital I2S output
- Commonly used with ESP32 and STM32 platforms

Product link:  
https://tronic.lk/product/inmp441-omnidirectional-microphone-module-i2s-interface

### ICS-43434 (Compatible)
- High-performance MEMS microphone
- I2S digital interface
- Pin-compatible and protocol-compatible with INMP441 for most use cases

Product page:  
https://invensense.tdk.com/products/ics-43434/

> ⚠️ Note: Minor configuration differences (clock polarity, bit depth, etc.) may exist depending on the platform and I2S peripheral implementation.

---

## 🧠 Supported Microcontrollers

### ESP32-WROOM-32
- Dual-core MCU with built-in I2S peripheral
- Ideal for audio streaming, recording, and DSP
- Used for:
  - Audio capture
  - Serial/USB debugging
  - Real-time experiments

### STM32L073RZ
- Ultra-low-power ARM Cortex-M0+
- Used to explore:
  - Low-power audio acquisition
  - I2S + DMA operation
  - Embedded audio buffering

---

## 🔌 Typical Wiring (I2S)

| Microphone Pin | ESP32 | STM32 |
|----------------|-------|-------|
| VDD            | 3.3V  | 3.3V  |
| GND            | GND   | GND   |
| WS (LRCLK)     | GPIO  | I2S_WS |
| SCK (BCLK)     | GPIO  | I2S_CK |
| SD             | GPIO  | I2S_SD |
| L/R Select     | GND or VDD | GND or VDD |

> Refer to individual project folders for exact pin assignments.

---

## 🛠 Tools & Frameworks Used

- **PlatformIO / Arduino (ESP32)**
- **STM32CubeIDE**
- **HAL drivers**
- Logic analyzer (for I2S debugging)
- Serial plotters / audio visualization tools

---

## 🧪 Experiments Included

- Basic I2S microphone reading
- Mono vs stereo configuration
- DMA-based audio buffering
- Sampling rate experiments
- Noise floor and gain testing
- Power consumption tests (STM32)

---

## 🚀 Getting Started

1. Clone the repository:
   ```bash
   git clone <your-repo-url>
   ```
2. Open the relevant folder for your MCU
3. Configure pins and I2S settings if needed
4. Build and flash the project
5. Monitor output via serial or debugger

---

## 📌 Notes

- This repository is **experimental and educational**
- Code may change frequently as new tests are added
- Contributions and suggestions are welcome

---

## ✨ Author

Experiments and documentation by the repository owner.  
Happy hacking with digital audio! 🎶
