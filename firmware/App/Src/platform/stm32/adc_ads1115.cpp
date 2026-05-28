// App/Src/system/adc_ads1115.cpp
#include "platform/stm32/adc_ads1115.hpp"
#include "system/i2c_arbiter.hpp"
#include "system/logger/logger.hpp"
#include "FreeRTOS.h"
#include "task.h"

namespace Hephaestus {

    bool Ads1115::init() {
        HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(_hi2c, _address, 3, 100);
        
        if (status == HAL_OK) {
            Logger::info("ADC", "ADS1115 found at address 0x%02X", _address);
            return true;
        } else {
            Logger::error("ADC", "ADS1115 NOT FOUND at address 0x%02X!", _address);
            return false;
        }
    }

    float Ads1115::readVoltage(uint8_t channel) {
        if (channel > 3) return 0.0f; 
        if (i2c1Mutex == nullptr) return 0.0f;

        float voltage = 0.0f;

        if (xSemaphoreTake(i2c1Mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            
            // --- Конфігурація регістра Config ---
            // Біт [15] = 1 (Почати одиночну конвертацію)
            // Біти [14:12] = MUX. 100 = AIN0, 101 = AIN1, 110 = AIN2, 111 = AIN3
            // Біти [11:9] = PGA. 001 = ±4.096V
            // Біт [8] = 1 (Single-shot mode)
            // Біти [7:5] = DR. 111 = 860 SPS
            // Біти [4:0] = COMP. 00011 = Вимкнути компаратор
            
            uint16_t mux = (0b100 + channel) << 12;
            uint16_t config = 0x8000 | mux | 0x0200 | 0x0100 | 0x00E0 | 0x0003;

            uint8_t configMsg[3];
            configMsg[0] = REG_POINTER_CONFIG;
            configMsg[1] = (config >> 8) & 0xFF; // MSB
            configMsg[2] = config & 0xFF;        // LSB

            HAL_I2C_Master_Transmit(_hi2c, _address, configMsg, 3, HAL_MAX_DELAY);

            vTaskDelay(pdMS_TO_TICKS(2));

            uint8_t pointerMsg[1] = { REG_POINTER_CONVERT };
            uint8_t data[2] = {0, 0};

            HAL_I2C_Master_Transmit(_hi2c, _address, pointerMsg, 1, HAL_MAX_DELAY);
            HAL_I2C_Master_Receive(_hi2c, _address, data, 2, HAL_MAX_DELAY);

            xSemaphoreGive(i2c1Mutex);

            int16_t raw_adc = (int16_t)((data[0] << 8) | data[1]);

            voltage = raw_adc * 0.000125f;

            if (voltage < 0.0f) voltage = 0.0f;
        }

        return voltage;
    }

} // namespace Hephaestus
