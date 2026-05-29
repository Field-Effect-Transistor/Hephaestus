//  App/Inc/platform/pc/hal_mock.hpp
#pragma once
#include <cstdint>

// Імітуємо типи та константи STM32 HAL
#define HAL_MAX_DELAY 0xFFFFFFFF
#define HAL_OK 0

struct I2C_HandleTypeDef {};
struct UART_HandleTypeDef { int gState; };
struct TIM_HandleTypeDef {
    struct { uint32_t Period; } Init;
};

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;

extern "C" {
    // Оголошуємо функції HAL
    uint32_t HAL_GetTick();
    int HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
}
