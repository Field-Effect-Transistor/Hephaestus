//  App/Inc/system/system_config.hpp
#pragma once
#include <cstdint>

namespace Hephaestus {

    struct SensorConfig {
        // --- Калібрування Блока Живлення ---
        float psuDividerRatio = 11.0f;     // Теоретично (100k + 10k) / 10k = 11.0
        float psuVoltageOffset = 0.0f;     // Корекція в Вольтах (якщо АЦП або резистори мають похибку)

        // --- Калібрування Холодного спаю (NTC) ---
        float ntcReferenceVoltage = 3.3f;  // Реальна напруга живлення дільника
        float ntcTopResistor = 10000.0f;   // Верхній резистор (10k)
        float ntcNominalRes = 10000.0f;    // Опір NTC при 25°C
        float ntcNominalTemp = 298.15f;    // 25°C у Кельвінах
        float ntcBeta = 3950.0f;           // B-коефіцієнт (Beta)

        // --- Калібрування Паяльника (T12) ---
        float ironOpAmpGain = 201.0f;      // 1 + (200k / 1k)
        float ironOpAmpOffsetV = 0.0f;     // Напруга зміщення ОП (зсув нуля), у Вольтах на вході АЦП
        float ironTcSensitivity = 0.041f;  // мВ на градус Цельсія (для типу К ~0.041)

        // --- Калібрування Термофена (Air) ---
        float airOpAmpGain = 101.0f;       // 1 + (100k / 1k) - Уточни за своєю схемою (наприклад, 100k і 1k)
        float airOpAmpOffsetV = 0.0f;      // Напруга зміщення ОП
        float airTcSensitivity = 0.041f;   // мВ на градус Цельсія
    };

    struct SystemConfig {
        SensorConfig sensors;
    };

} // namespace Hephaestus
