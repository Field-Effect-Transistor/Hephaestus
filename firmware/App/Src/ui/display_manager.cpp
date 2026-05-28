#include "ui/display_manager.hpp"
#include "system/logger/logger.hpp"
#include <cstdio>

// Підключаємо платформо-залежні речі
#ifndef PC_SIMULATOR
    #include "i2c.h"
    #include "system/i2c_arbiter.hpp"
    #include "FreeRTOS.h"
    #include "task.h"
    #define OLED_I2C_ADDRESS 0x78 
#else
    #include "platform/pc/sdl_context.hpp"
    // Отримуємо доступ до глобального контексту вікна з main_pc.cpp
    extern Hephaestus::SdlContext sdlContext; 
#endif

extern "C" {
    // Коллбек затримок для u8g2
    uint8_t u8x8_gpio_and_delay_stm32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
#ifndef PC_SIMULATOR
        switch (msg) {
            case U8X8_MSG_DELAY_MILLI:
                vTaskDelay(pdMS_TO_TICKS(arg_int));
                break;
            default: return 0;
        }
#endif
        return 1;
    }

    // Коллбек I2C шини для u8g2
    uint8_t u8x8_byte_stm32_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
#ifndef PC_SIMULATOR
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
                    HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS, buffer, buf_idx, 10);
                    xSemaphoreGive(Hephaestus::i2c1Mutex);
                } else {
                    HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS, buffer, buf_idx, 10);
                }
                break;
            }
            default: return 0;
        }
#endif
        return 1; // У симуляторі I2C "завжди працює"
    }
}

namespace Hephaestus {

    void DisplayManager::init() {
        u8g2_Setup_ssd1306_i2c_128x64_noname_f(
            &_u8g2, 
            U8G2_R0, 
            u8x8_byte_stm32_hw_i2c, 
            u8x8_gpio_and_delay_stm32
        );

        u8g2_InitDisplay(&_u8g2);
        u8g2_SetPowerSave(&_u8g2, 0);
    }

    void DisplayManager::update(const HeaterChannel& iron, const HeaterChannel& air) {
        u8g2_ClearBuffer(&_u8g2);

        char textBuffer[16]; 
        uint8_t right_offset = 70;

        u8g2_DrawVLine(&_u8g2, 64, 0, 64);
        
        // --- Air Channel ---
        u8g2_SetFont(&_u8g2, u8g2_font_helvB08_tf);
        u8g2_DrawStr(&_u8g2, 0, 10, "AIR SET:");
        snprintf(textBuffer, sizeof(textBuffer), "%d", air.getTargetTemp());
        u8g2_DrawStr(&_u8g2, 0, 22, textBuffer);

        u8g2_SetFont(&_u8g2, u8g2_font_logisoso24_tn); 
        snprintf(textBuffer, sizeof(textBuffer), "%d", air.getCurrentTemp());
        u8g2_DrawStr(&_u8g2, 0, 52, textBuffer);

        u8g2_SetFont(&_u8g2, u8g2_font_helvB08_tf);
        const char* airState = (air.getState() == ChannelState::Active) ? "ACTIVE" : 
                               (air.getState() == ChannelState::Sleep) ? "SLEEP" : "OFF";
        u8g2_DrawStr(&_u8g2, 0, 64, airState);
        
        // --- Iron Channel ---
        u8g2_SetFont(&_u8g2, u8g2_font_helvB08_tf);
        u8g2_DrawStr(&_u8g2, right_offset, 10, "IRON SET:");
        snprintf(textBuffer, sizeof(textBuffer), "%d", iron.getTargetTemp());
        u8g2_DrawStr(&_u8g2, right_offset, 22, textBuffer);

        u8g2_SetFont(&_u8g2, u8g2_font_logisoso24_tn);
        snprintf(textBuffer, sizeof(textBuffer), "%d", iron.getCurrentTemp());
        u8g2_DrawStr(&_u8g2, right_offset, 52, textBuffer);

        u8g2_SetFont(&_u8g2, u8g2_font_helvB08_tf);
        const char* ironState = (iron.getState() == ChannelState::Active) ? "ACTIVE" : 
                                (iron.getState() == ChannelState::Sleep) ? "SLEEP" : "OFF";
        u8g2_DrawStr(&_u8g2, right_offset, 64, ironState);

#ifndef PC_SIMULATOR
        // Відправляємо кадр на справжній дисплей по I2C
        u8g2_SendBuffer(&_u8g2);
#else
        // Передаємо кадр у SDL-адаптер для малювання на екрані ПК
        sdlContext.submitBuffer(u8g2_GetBufferPtr(&_u8g2));
#endif
    }
} // namespace Hephaestus
