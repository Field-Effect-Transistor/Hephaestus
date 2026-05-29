//  App/Inc/platform/stm32/pwm_driver_soft.hpp
#pragma once
#include "interfaces/IPwm.hpp"
#include "gpio.h"
#include "FreeRTOS.h"
#include "task.h"

namespace Hephaestus {
    class PwmDriverSoft : public IPwm {
    private:
        GPIO_TypeDef* _port;
        uint16_t      _pin;
        float         _dutyCycle = 0.0f;
        bool          _enabled = false;
        
        // Для керування симістором 220В використовуємо період 1 секунда (1000 мс).
        // 50% ШІМ = 500 мс увімкнено, 500 мс вимкнено.
        static constexpr uint32_t PERIOD_MS = 1000; 

    public:
        PwmDriverSoft(GPIO_TypeDef* port, uint16_t pin) : _port(port), _pin(pin) {
            HAL_GPIO_WritePin(_port, _pin, GPIO_PIN_RESET);
        }

        void enable(bool state) override {
            _enabled = state;
            if (!state) HAL_GPIO_WritePin(_port, _pin, GPIO_PIN_RESET);
        }

        void setDutyCycle(float percent) override {
            if (percent < 0.0f) percent = 0.0f;
            if (percent > 100.0f) percent = 100.0f;
            _dutyCycle = percent;
        }

        float getDutyCycle() const override { return _dutyCycle; }

        // Цей метод ми викличемо у controlLoopTask (20 разів на сек)
        void tick(uint32_t currentTickMs) {
            if (!_enabled || _dutyCycle <= 0.0f) {
                HAL_GPIO_WritePin(_port, _pin, GPIO_PIN_RESET);
                return;
            }
            if (_dutyCycle >= 100.0f) {
                HAL_GPIO_WritePin(_port, _pin, GPIO_PIN_SET);
                return;
            }

            uint32_t phase = currentTickMs % PERIOD_MS;
            uint32_t threshold = (uint32_t)((_dutyCycle / 100.0f) * PERIOD_MS);

            if (phase < threshold) {
                HAL_GPIO_WritePin(_port, _pin, GPIO_PIN_SET);
            } else {
                HAL_GPIO_WritePin(_port, _pin, GPIO_PIN_RESET);
            }
        }
    };
}
