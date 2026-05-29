//  App/Src/platform/pc/hal_mock.cpp
#include "platform/pc/hal_mock.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include <iostream>
#include <chrono>

// Ініціалізуємо глобальні змінні
UART_HandleTypeDef huart1 = {0}; 
I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim1 = {{1000}};
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;

extern "C" {
    uint32_t HAL_GetTick() {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    }

    int HAL_UART_Transmit(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size, uint32_t Timeout) {
        for(uint16_t i = 0; i < Size; i++) std::cout << (char)pData[i];
        return HAL_OK; 
    }

    // ТУТ МАГІЯ СИМУЛЯЦІЇ: Імітуємо реальну затримку передачі байтів по I2C!
    int HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout) { 
        vTaskDelay(pdMS_TO_TICKS(1)); // Відправка 32 байтів займає ~1 мс
        return HAL_OK; 
    }

    int HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout) { 
        vTaskDelay(pdMS_TO_TICKS(1)); 
        return HAL_OK; 
    }
    
    int HAL_I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout) { return HAL_OK; }
    int HAL_GPIO_ReadPin(void* GPIOx, uint16_t GPIO_Pin) { return 1; }
    void HAL_TIM_Encoder_Start(TIM_HandleTypeDef *htim, uint32_t Channel) {}
    uint32_t __HAL_TIM_GET_COUNTER(TIM_HandleTypeDef *htim) { return 0; }
    void HAL_TIM_PWM_Start(TIM_HandleTypeDef *htim, uint32_t Channel) {}
    void HAL_TIM_PWM_Stop(TIM_HandleTypeDef *htim, uint32_t Channel) {}
    void __HAL_TIM_SET_COMPARE(TIM_HandleTypeDef *htim, uint32_t Channel, uint32_t value) {}
}
