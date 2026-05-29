//  App/Inc/system/math_sensors.hpp

#pragma once

#include <cmath>
#include "system/system_config.hpp"

namespace Hephaestus {

    class MathSensors {
    public:
        static float calculatePsuVoltage(float adcVolts, const SensorConfig& cfg) {
            float voltage = (adcVolts * cfg.psuDividerRatio) + cfg.psuVoltageOffset;
            return (voltage > 0.0f) ? voltage : 0.0f;
        }

        static float calculateNtcTempC(float adcVolts, const SensorConfig& cfg) {
            if (adcVolts <= 0.01f || adcVolts >= (cfg.ntcReferenceVoltage - 0.01f)) {
                return 25.0f;
            }

            float rNtc = cfg.ntcTopResistor * adcVolts / (cfg.ntcReferenceVoltage - adcVolts);

            float tempK = rNtc / cfg.ntcNominalRes;
            tempK = std::log(tempK);
            tempK /= cfg.ntcBeta;
            tempK += (1.0f / cfg.ntcNominalTemp);
            tempK = 1.0f / tempK;

            return tempK - 273.15f;
        }

        static float calculateThermocoupleTemp(
            float adcVolts,
            float opAmpGain,
            float opAmpOffsetV,
            float coldJunctionTempC) 
        {
            float correctedVolts = adcVolts - opAmpOffsetV;
            if (correctedVolts < 0.0f) correctedVolts = 0.0f;

            float tcVolts = correctedVolts / opAmpGain;
            float mv = tcVolts * 1000.0f;

            // Спрощений поліном для термопари К-типу (0 - 500C)
            // Т = 25.0836 * V - 0.0786 * V^2 + 0.2503 * V^3
            // Оптимізовано схемою Горнера для зменшення кількості множень у STM32
            float tcTempC = mv * (25.0836f + mv * (-0.0786f + 0.2503f * mv));

            return coldJunctionTempC + tcTempC;
        }

        static float applyCalibration(float rawTemp, const float calRaw[3], const float calReal[3]) {
            // Якщо температура нижча за першу точку (екстраполяція вниз)
            if (rawTemp <= calRaw[0]) {
                float k = (calReal[1] - calReal[0]) / (calRaw[1] - calRaw[0]);
                return calReal[0] + k * (rawTemp - calRaw[0]);
            }
            // Якщо температура між 1 і 2 точкою
            else if (rawTemp <= calRaw[1]) {
                float k = (calReal[1] - calReal[0]) / (calRaw[1] - calRaw[0]);
                return calReal[0] + k * (rawTemp - calRaw[0]);
            }
            // Якщо температура між 2 і 3 точкою
            else if (rawTemp <= calRaw[2]) {
                float k = (calReal[2] - calReal[1]) / (calRaw[2] - calRaw[1]);
                return calReal[1] + k * (rawTemp - calRaw[1]);
            }
            // Якщо вище 3 точки (екстраполяція вгору)
            else {
                float k = (calReal[2] - calReal[1]) / (calRaw[2] - calRaw[1]);
                return calReal[2] + k * (rawTemp - calRaw[2]);
            }
        }
    };

} // namespace Hephaestus