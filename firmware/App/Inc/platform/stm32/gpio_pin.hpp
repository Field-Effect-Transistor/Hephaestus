//  App/Inc/platform/stm32/gpio_pin.hpp

#pragma once
#include "interfaces/IDigitalPin.hpp"
#include "gpio.h"

namespace Hephaestus {
    class GpioPin : public IDigitalPin {
    private:
        GPIO_TypeDef* _port;
        uint16_t _pin;
        bool _activeLow;

    public:
        GpioPin(GPIO_TypeDef* port, uint16_t pin, bool activeLow = true) 
            : _port(port), _pin(pin), _activeLow(activeLow) {}

        bool isActive() override {
            bool state = (HAL_GPIO_ReadPin(_port, _pin) == GPIO_PIN_SET);
            return _activeLow ? !state : state;
        }
    };
}
