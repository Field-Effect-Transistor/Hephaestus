// App/Inc/system/system_config.hpp
#pragma once
#include <cstdint>

namespace Hephaestus {

    struct SensorConfig {
        //  --- Попередні налаштування для PSU та NTC
        float psuDividerRatio = 11.0f;     
        float psuVoltageOffset = 0.0f;     
        float ntcReferenceVoltage = 3.3f;  
        float ntcTopResistor = 10000.0f;   
        float ntcNominalRes = 10000.0f;    
        float ntcNominalTemp = 298.15f;    
        float ntcBeta = 3950.0f;           

        //  --- Калібрування Паяльника (T12) ---
        float ironOpAmpGain = 201.0f;      
        float ironOpAmpOffsetV = 0.0f;     
        float ironTcSensitivity = 0.041f;  
        
        // ПІД КОЕФІЦІЄНТИ (Розраховані за Зіглером-Нікольсом у Розділі 1)
        float ironKp = 6.0f;
        float ironKi = 30.534f;
        float ironKd = 0.295f;

        // --- Калібрування Термофена (Air) ---
        float airOpAmpGain = 101.0f;       
        float airOpAmpOffsetV = 0.0f;      
        float airTcSensitivity = 0.041f;   
        
        // ПІД КОЕФІЦІЄНТИ ДЛЯ ФЕНА
        float airKp = 2.0f;
        float airKi = 5.0f;
        float airKd = 0.1f;

        // --- Налаштування Сну ---
        uint16_t ironSleepTimeoutSec = 180; // 3 хвилини (180 сек) бездіяльності для паяльника
        uint16_t airSleepTimeoutSec = 300;  // 5 хвилин (300 сек) бездіяльності для фена
    };

    struct SystemConfig {
        SensorConfig sensors;
    };

} // namespace Hephaestus