// App/Inc/platform/stm32/encoder_tim.hpp

#pragma once

#include <cstdint>
#include "main.h"
#include "interfaces/IEncoder.hpp"

namespace Hephaestus {

    class EncoderTim : public IEncoder {
    private:
        TIM_HandleTypeDef* _htim;
        int16_t _lastCount = 0;
        
        const int16_t ACCEL_THRESHOLD = 2;
        const int16_t ACCEL_MULTIPLIER = 5;

    public:
        explicit EncoderTim(TIM_HandleTypeDef* htim) : _htim(htim) {}

        void init() override;
        EncoderResult getSteps() override;
    };

} // namespace Hephaestus
