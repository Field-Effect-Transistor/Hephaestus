//  App/Inc/platform/stm32/pwm_driver_tim.hpp
#pragma once
#include "interfaces/IPwm.hpp"
#include "tim.h" // Для TIM_HandleTypeDef

namespace Hephaestus {
    class PwmDriverTim : public IPwm {
    private:
        TIM_HandleTypeDef* _htim;
        uint32_t _channel;

    public:
        PwmDriverTim(TIM_HandleTypeDef* htim, uint32_t channel) 
            : _htim(htim), _channel(channel) {}

        void enable(bool state) override {
            if (state) {
                HAL_TIM_PWM_Start(_htim, _channel);
            } else {
                HAL_TIM_PWM_Stop(_htim, _channel);
                __HAL_TIM_SET_COMPARE(_htim, _channel, 0);
            }
        }

        void setDutyCycle(float percent) override {
            if (percent < 0.0f) percent = 0.0f;
            if (percent > 100.0f) percent = 100.0f;

            uint32_t compareValue = (uint32_t)((percent / 100.0f) * (_htim->Init.Period + 1));
            
            __HAL_TIM_SET_COMPARE(_htim, _channel, compareValue);
        }
    };
}
