//  App/Src/system/encoder.cpp
#include "system/encoder.hpp"

namespace Hephaestus {
    void Encoder::init() {
        HAL_TIM_Encoder_Start(_htim, TIM_CHANNEL_ALL);
        _lastCount = (int16_t)__HAL_TIM_GET_COUNTER(_htim);
    }

    EncoderResult Encoder::getSteps() {
    int16_t currentCount = (int16_t)__HAL_TIM_GET_COUNTER(_htim);
    int16_t diff = currentCount - _lastCount;
    EncoderResult res = {0, 0};

    if (diff >= 4 || diff <= -4) {
        int16_t raw_steps = diff / 4; 
        _lastCount += (raw_steps * 4);

        res.raw = raw_steps;

        int16_t abs_steps = raw_steps > 0 ? raw_steps : -raw_steps;
        
        if (abs_steps >= ACCEL_THRESHOLD) {
            res.accelerated = raw_steps * ACCEL_MULTIPLIER;
        } else {
            res.accelerated = raw_steps;
        }
    }

    return res;
}
}   //  namespace   Hephaestus

