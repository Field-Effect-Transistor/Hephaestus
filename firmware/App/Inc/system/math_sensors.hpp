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
            float sensitivity, float coldJunctionTempC) 
        {
            float correctedVolts = adcVolts - opAmpOffsetV;
            if (correctedVolts < 0.0f) correctedVolts = 0.0f;

            float tcVolts = correctedVolts / opAmpGain;
            
            float tcMillivolts = tcVolts * 1000.0f;

            float tcTempC = tcMillivolts / sensitivity; 

            return coldJunctionTempC + tcTempC;
        }
    };

} // namespace Hephaestus
