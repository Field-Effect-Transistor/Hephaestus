// App/Inc/system/adc_ads1115.hpp
#pragma once

#include "system/IAdc.hpp"
#include "i2c.h" // Для I2C_HandleTypeDef

namespace Hephaestus {

    class Ads1115 : public IAdc {
    private:
        I2C_HandleTypeDef* _hi2c;
        uint8_t _address;

        // Регістри ADS1115
        static constexpr uint8_t REG_POINTER_CONVERT = 0x00;
        static constexpr uint8_t REG_POINTER_CONFIG  = 0x01;

    public:
        // Адреса за замовчуванням 0x48. Для STM32 HAL її треба зсунути вліво на 1 біт (0x48 << 1 = 0x90)
        explicit Ads1115(I2C_HandleTypeDef* hi2c, uint8_t address = 0x90) 
            : _hi2c(hi2c), _address(address) {}

        // Перевіряє наявність АЦП на шині
        bool init();

        // Реалізація інтерфейсу IAdc
        float readVoltage(uint8_t channel) override;
    };

} // namespace Hephaestus
