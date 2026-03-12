# ESP32-Line-Following-Robot
An optimized autonomous robot built for speed and reliability. Leveraging the ESP32’s high clock speed for near-instantaneous sensor-to-motor reaction times, this bot utilizes a multi-stage logic gate system to navigate complex tracks without the overhead of complex tuning.
# 🏎️ Line Follower
**An ESP32-powered racing robot engineered for precision and velocity.**

It is a high-performance autonomous line follower. By combining the processing power of the **ESP32** with the **QTR-8RC** reflectance sensor array and a custom **PID control loop**, SantiBot achieves smooth, high-speed navigation on complex tracks.

---

## 📽️ Performance Demo

https://github.com/user-attachments/assets/07834bde-72f6-4334-bd97-bfd6258b83fe

> **Note:** Above is a demonstration of SantiBot navigating a technical track using real-time PID corrections.

---

## 🛠️ Hardware Architecture

### Core Components
* **Brain:** ESP32 DevKit (30/38-pin) - Dual-core power for high-frequency PID cycles.
* **Vision:** QTR-8RC (8-Channel Reflectance Sensor) - RC timing method for high-resolution line detection.
* **Power:** 2S Li-ion Battery (7.4V) - High discharge rate for rapid motor response.
* **Regulation:** LM2596 Buck Converter - Stepping down to **5V** for stable sensor logic.
* **Chassis:** 4-Wheel Drive + Low-friction Caster/Chassis for high-speed maneuvering.

### 🔌 Pin Mapping (Wiring Diagram)

To ensure the PID logic functions correctly, the sensors must be wired in the exact order defined in the code array: `{4, 23, 2, 5, 18, 19, 21, 22}`.



#### 1. Sensor Data Pins (Signal)
| Sensor Position | QTR-8RC Pin | ESP32 GPIO |
| :--- | :--- | :--- |
| **Far Left** | Pin 1 | **GPIO 4** |
| **Mid Left** | Pin 2 | **GPIO 23** |
| **Inner Left** | Pin 3 | **GPIO 2** |
| **Center Left** | Pin 4 | **GPIO 5** |
| **Center Right** | Pin 5 | **GPIO 18** |
| **Inner Right** | Pin 6 | **GPIO 19** |
| **Mid Right** | Pin 7 | **GPIO 21** |
| **Far Right** | Pin 8 | **GPIO 22** |

#### 2. Power & Control
| QTR Pin | Connection | Note |
| :--- | :--- | :--- |
| **VCC** | **Buck Converter (5V)** | Provides higher signal-to-noise ratio than 3.3V |
| **GND** | **ESP32 GND** | Use Star-Grounding to prevent motor noise |
| **LED ON** | **ESP32 3.3V/5V** | Keeps Infrared LEDs active for detection |

---

## 🧠 Control Logic: The PID Loop

SantiBot utilizes a Proportional-Integral-Derivative algorithm to maintain its position on the line. This allows the robot to predict turns and dampen oscillations at high speeds.

$$Output = (K_p \cdot error) + (K_i \cdot \int error \, dt) + (K_d \cdot \frac{de}{dt})$$



* **Proportional ($K_p$):** Reacts to the current error (distance from center).
* **Integral ($K_i$):** Corrects accumulated steady-state errors.
* **Derivative ($K_d$):** Predicts future error to dampen "fishtailing" at high velocity.

---

## ⚡ Electrical Best Practices

* **Capacitance Management:** The "RC" reading method relies on timing. Keep sensor wires as short as possible to avoid parasitic capacitance.
* **EMI Shielding:** Twist the **VCC** and **GND** wires leading to the motors to cancel out electromagnetic interference that can cause sensor "jitter."
* **Voltage Stability:** Always use the Buck Converter for the sensors. Drawing 5V directly from the ESP32 while motors are under load can cause the MCU to brown out or the sensors to lose calibration.

---

## 🚀 Getting Started

1.  **Assemble:** Connect the hardware according to the [Pin Mapping](#1-sensor-data-pins-signal).
2.  **Upload:** Flash the `Automatic.ino` sketch using the Arduino IDE.
3.  **Calibrate:** During the first 5 seconds of power-up, physically move the robot side-to-side over the line to calibrate the min/max reflectance values.
4.  **Bluetooth Upload:** Flash the `BluetoothEnable.ino` sketch using the Arduino IDE.
5.  **Telemetry:** Use the **Bluetooth Electronics App** to monitor real-time sensor data and fine-tune PID constants without a USB cable.

---

## 📱 Remote Interface
ESP32 line follower is compatible with the **Bluetooth Electronics** mobile app. Use the custom dashboard to:
* Start/Stop the race.
* Adjust all the essential needs in the app and the Esp32 connects it to your device

---
## 🤝 Contact & Connect

**Project Lead:** Kunal Gupta

Feel free to reach out for collaborations, troubleshooting, or just to talk robotics!

* **📧 Email:** kunalgupta3212@gmail.com
* **📸 Instagram:** @kunal._.gupta
* **💻 GitHub:** [github.com/kunalgupta32]

---
Developed with ❤️ for the Robotics Community.
