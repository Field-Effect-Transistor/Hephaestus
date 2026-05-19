//  App/Inc/system/encoder.hpp
#pragma once

#include <cstdint>
#include "main.h"

namespace Hephaestus {
    struct EncoderResult {
        int16_t raw;
        int16_t accelerated;
        
        bool hasMovement() const { return raw != 0; }
    };

    class Encoder {
    private:
        TIM_HandleTypeDef* _htim;
        int16_t _lastCount = 0;
        
        const int16_t ACCEL_THRESHOLD = 2;
        const int16_t ACCEL_MULTIPLIER = 5;

    public:
        explicit Encoder(TIM_HandleTypeDef* htim) : _htim(htim) {}

        void init();
        EncoderResult getSteps();
    };

} // namespace Hephaestus
