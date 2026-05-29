#include "ui/display_manager.hpp"
#include "system/logger/logger.hpp"
#include "system/i2c_arbiter.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include <cstdio>

#ifndef PC_SIMULATOR
    #include "i2c.h"
#else
    #include "platform/pc/hal_mock.hpp"
    #include "platform/pc/sdl_context.hpp"
    extern Hephaestus::SdlContext sdlContext; 
#endif

#define OLED_I2C_ADDRESS 0x78 

extern "C" {
    uint8_t u8x8_gpio_and_delay_stm32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
        if (msg == U8X8_MSG_DELAY_MILLI) {
            vTaskDelay(pdMS_TO_TICKS(arg_int));
        }
        return 1;
    }

    uint8_t u8x8_byte_stm32_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
        static uint8_t buffer[32];
        static uint8_t buf_idx;

        switch (msg) {
            case U8X8_MSG_BYTE_SEND:
                for (uint8_t i = 0; i < arg_int; i++) buffer[buf_idx++] = ((uint8_t *)arg_ptr)[i];
                break;
            case U8X8_MSG_BYTE_INIT:
            case U8X8_MSG_BYTE_SET_DC:
                break;
            case U8X8_MSG_BYTE_START_TRANSFER:
                buf_idx = 0;
                break;
            case U8X8_MSG_BYTE_END_TRANSFER:
            {
                if (Hephaestus::i2c1Mutex != nullptr) {
                    xSemaphoreTake(Hephaestus::i2c1Mutex, portMAX_DELAY);
                    HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS, buffer, buf_idx, HAL_MAX_DELAY);
                    xSemaphoreGive(Hephaestus::i2c1Mutex);
                } else {
                    HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS, buffer, buf_idx, HAL_MAX_DELAY);
                }
                break;
            }
            default:
                return 0;
        }
        return 1;
    }
}

namespace Hephaestus {

    void DisplayManager::init(IScreen* initialScreen, SystemContext* ctx) {
        _currentScreen = initialScreen;
        _context = ctx;

        u8g2_Setup_ssd1306_i2c_128x64_noname_f(
            &_u8g2, U8G2_R0, u8x8_byte_stm32_hw_i2c, u8x8_gpio_and_delay_stm32
        );

        u8g2_InitDisplay(&_u8g2);
        u8g2_SetPowerSave(&_u8g2, 0);
    }

    void DisplayManager::update() {
        if (!_currentScreen || !_context) return;

        u8g2_ClearBuffer(&_u8g2);
        _currentScreen->draw(&_u8g2, *_context);

#ifndef PC_SIMULATOR
        u8g2_SendBuffer(&_u8g2);
#else
        u8g2_SendBuffer(&_u8g2);
        
        sdlContext.submitBuffer(u8g2_GetBufferPtr(&_u8g2));
#endif
    }

    void DisplayManager::dispatchEncoder(int16_t steps) {
        if (_currentScreen && _context) _currentScreen->handleEncoder(steps, *_context);
    }

    void DisplayManager::dispatchButton(ButtonEvent event) {
        if (!_currentScreen || !_context) return;
        IScreen* next = _currentScreen->handleButton(event, *_context);
        if (next) _currentScreen = next;
    }

} // namespace Hephaestus
