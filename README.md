# Smart IoT Soldering Station (T12 + 858D)

**Бакалаврська кваліфікаційна робота**
Розробка двоканальної паяльної станції на базі мікроконтролера ESP32-S3 з функцією віддаленого керування та телеметрії.

![Status](https://img.shields.io/badge/Status-In%20Development-yellow)
![Platform](https://img.shields.io/badge/Platform-ESP32-blue)
![License](https://img.shields.io/badge/License-MIT-green)

## 📋 Основні можливості
*   **Два незалежні канали:** Паяльник (жало T12) та Термофен (858D).
*   **Точність:** ПІД-регулювання температури з використанням зовнішнього 16-бітного АЦП.
*   **IoT функціонал:** Web-інтерфейс для налаштування профілів, калібрування та перегляду графіків нагріву (WebSocket).
*   **OS:** Використання FreeRTOS для розподілу задач між ядрами (Core 0: Wi-Fi/Web, Core 1: PID/Sensors).
*   **Інтерфейс:** OLED дисплей + 2 енкодери для зручного керування.

## 🛠 Технічний стек

### Апаратна частина (Hardware)
*   **MCU:** ESP32-S3-WROOM-1 (N16R8)
*   **ADC:** ADS1115 (16-bit, I2C)
*   **OpAmp:** MCP6002 (Rail-to-Rail)
*   **Power:** 24V 4A DC Power Supply
*   **Drivers:** MOSFET (T12/Fan), Triac BTA16 + Optocoupler (Hot Air Heater)

### Програмна частина (Firmware)
*   **IDE:** VS Code + PlatformIO
*   **Framework:** Arduino
*   **OS:** FreeRTOS

## 📂 Структура репозиторію

```text
├── firmware/      # Вихідний код (PlatformIO проєкт)
│   ├── src/       # Основні файли (.cpp)
│   └── lib/       # Локальні бібліотеки (PID, Display, Web)
│
├── hardware/      # Схемотехніка та плати (KiCad 7.0)
│   ├── schematics # Принципові схеми
│   └── pcb        # Трасування плати
│
├── mechanical/    # 3D-моделі корпусу (STL/STEP)
└── docs/          # Документація, даташити та розрахунки
