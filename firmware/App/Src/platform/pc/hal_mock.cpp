#include <iostream>
#include <chrono>
#include <cstdint>

// Імітуємо структури HAL
struct I2C_HandleTypeDef {};
struct UART_HandleTypeDef { int gState; };
struct TIM_HandleTypeDef {
    struct { uint32_t Period; } Init;
};

UART_HandleTypeDef huart1 = {0}; // 0 = HAL_UART_STATE_READY
I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim1 = {{1000}};
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;

extern "C" {
    // Імітація часу
    uint32_t HAL_GetTick() {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    }

    // Імітація виводу в UART (Друкуємо прямо в консоль Linux)
    int HAL_UART_Transmit(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size, uint32_t Timeout) {
        for(uint16_t i = 0; i < Size; i++) {
            std::cout << (char)pData[i];
        }
        return 0; // HAL_OK
    }

    // Інші заглушки, щоб лінкер не сварився
    int HAL_I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout) { return 0; }
    int HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout) { return 0; }
    int HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout) { return 0; }
    
    int HAL_GPIO_ReadPin(void* GPIOx, uint16_t GPIO_Pin) { return 1; } // Кнопки не натиснені
    void HAL_TIM_Encoder_Start(TIM_HandleTypeDef *htim, uint32_t Channel) {}
    uint32_t __HAL_TIM_GET_COUNTER(TIM_HandleTypeDef *htim) { return 0; }
    
    void HAL_TIM_PWM_Start(TIM_HandleTypeDef *htim, uint32_t Channel) {}
    void HAL_TIM_PWM_Stop(TIM_HandleTypeDef *htim, uint32_t Channel) {}
    void __HAL_TIM_SET_COMPARE(TIM_HandleTypeDef *htim, uint32_t Channel, uint32_t value) {}
}
