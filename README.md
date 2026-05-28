***

# Hephaestus: Smart Soldering Station (T12 + 858D)

**Бакалаврська кваліфікаційна робота**  
Розробка мікрокомп'ютерної системи керування двоканальною паяльною станцією з прецизійним терморегулюванням та апаратною відмовобезпекою.

![Status](https://img.shields.io/badge/Status-In%20Development-yellow)
![Platform](https://img.shields.io/badge/Platform-STM32F103-blue)
![C++](https://img.shields.io/badge/C++-17-green)
![License](https://img.shields.io/badge/License-MIT-green)

## 📋 Основні можливості
*   **Два незалежні канали:** Контактний паяльник (жало T12) та компресорний термофен (858D).
*   **Прецизійна точність:** ПІД-регулювання з використанням зовнішнього 16-бітного АЦП та підсилювачів Zero-Drift.
*   **Апаратна безпека:** Електромеханічне реле із системою "самопідхвату" для захисту від термічного пробою.
*   **Багатозадачність (RTOS):** Використання FreeRTOS із витискальною багатозадачністю для суворого дотримання таймінгів алгоритму часового розділення (Time-Division).
*   **HIL Simulation:** Архітектура коду (Dependency Inversion) дозволяє компілювати та тестувати бізнес-логіку станції як нативну Linux/PC програму (Симулятор).
*   **Інтерфейс:** 1.3" OLED дисплей (u8g2) та апаратне декодування квадратурних енкодерів.

## 🛠 Технічний стек

### Апаратна частина (Hardware)
*   **MCU:** STM32F103C8T6 (ARM Cortex-M3)
*   **ADC:** ADS1115 (16-bit, I2C, ΔΣ)
*   **OpAmp:** OPA2333 / MCP6002
*   **Power:** Багаторівнева система живлення (24V, 5V, 3.3V Digital, 3.3V Analog)
*   **Drivers:** MOSFET (T12 / Турбіна), Triac BTA16 + Optocoupler (Нагрівач 220В)
*   **EDA:** KiCad 8.0

### Програмна частина (Firmware)
*   **Language:** C++17 / C
*   **Framework:** STM32 HAL / LL
*   **OS:** FreeRTOS (ARM CM3 Port / POSIX Port для симулятора)
*   **Build System:** CMake + Ninja
*   **IDE:** Visual Studio Code (C/C++ Extension Pack)

## 📂 Структура репозиторію

```text
├── firmware/
│   ├── App/             # Бізнес-логіка, Інтерфейси (C++)
│   │   ├── interfaces/  # Апаратні абстракції (Hardware Agnostic)
│   │   ├── logic/       # ПІД-регулятор, автомати станів кнопок
│   │   ├── platform/    # Реалізація інтерфейсів (STM32 HAL або PC Mocks)
│   │   └── ui/          # Керування дисплеєм (u8g2)
│   ├── Core/            # Згенерований CubeMX код (main.c, freertos.c)
│   ├── Drivers/         # STM32 HAL & CMSIS
│   └── Libs/            # Сторонні бібліотеки (u8g2, FreeRTOS POSIX)
│
├── hardware/            # Схемотехніка та плати (KiCad)
└── docs/                # Пояснювальна записка та даташити
```

## 🚀 Як зібрати проєкт

Проєкт підтримує дві цілі збірки: **Прошивка для мікроконтролера (STM32)** та **Симулятор для комп'ютера (Linux/PC)**.

### 1. Збірка Симулятора (Linux)
Симулятор компілює всю бізнес-логіку, запускає FreeRTOS поверх Pthreads операційної системи Linux та імітує датчики за допомогою об'єктів-заглушок (Mocks). Це дозволяє тестувати систему без реальної плати.

**Вимоги:** `cmake`, `ninja`, `gcc`, `g++`.

```bash
cd firmware
mkdir -p build_pc
rm -rf build_pc/* 

# Генеруємо конфігурацію для симулятора
cmake -DBUILD_SIMULATOR=ON -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ . -G Ninja -B build_pc   

# Збираємо проєкт
ninja -C build_pc 

# Запускаємо симулятор
./build_pc/Hephaestus_Sim
```

### 2. Збірка Прошивки (STM32)
Для крос-компіляції під мікроконтролер використовується офіційний Toolchain.

**Вимоги:** `arm-none-eabi-gcc`, `cmake`, `ninja`.

```bash
cd firmware
mkdir -p build
rm -rf build/*

# Генеруємо конфігурацію для крос-компіляції
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake . -G Ninja -B build

# Збираємо прошивку (.elf, .bin, .hex)
ninja -C build
```

*** 