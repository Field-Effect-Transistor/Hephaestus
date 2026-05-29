//  App/Src/platform/stm32/app.cpp

#include "app.hpp"
#include "usart.h"
#include "tim.h"
#include "i2c.h"
#include "iwdg.h"

#include "system/logger/logger.hpp"
#include "system/logger/uart_log_sink.hpp"
#include "system/i2c_arbiter.hpp"

#include "platform/stm32/adc_ads1115.hpp"
#include "platform/stm32/pwm_driver_tim.hpp"
#include "platform/stm32/pwm_driver_soft.hpp" 
#include "platform/stm32/gpio_pin.hpp"
#include "platform/stm32/encoder_tim.hpp"

#include "logic/station_manager.hpp"

#include "platform/stm32/flash_storage.hpp"

SemaphoreHandle_t Hephaestus::i2c1Mutex = nullptr;

// Hardware drivers instantiation
static Hephaestus::UARTLogSink uartLogSink(&huart1);
static Hephaestus::Ads1115     adc(&hi2c1);

static Hephaestus::GpioPin ironPin(GPIOB, GPIO_PIN_12);
static Hephaestus::GpioPin airPin(GPIOB, GPIO_PIN_13);
static Hephaestus::GpioPin startSensePin(GPIOB, GPIO_PIN_14, false);

static Hephaestus::EncoderTim ironEncoder(&htim2);
static Hephaestus::EncoderTim airEncoder(&htim4);

// ПРАВИЛЬНЕ ПРИЗНАЧЕННЯ:
// 1. Паяльник (T12) - Апаратний ШІМ (PA8)
static Hephaestus::PwmDriverTim ironPwm(&htim1, TIM_CHANNEL_1); 

// 2. Вентилятор Фена (Air Fan) - Апаратний ШІМ (PA11)
static Hephaestus::PwmDriverTim airFanPwm(&htim1, TIM_CHANNEL_4); 

// 3. Зумер (Buzzer) - Апаратний ШІМ (PB0)
static Hephaestus::PwmDriverTim buzzerPwm(&htim3, TIM_CHANNEL_3); 

// 4. Нагрівач Фена 220В (Air Heater Coil) - Програмний ШІМ (PB11)
static Hephaestus::PwmDriverSoft airPwm(GPIOB, GPIO_PIN_11); 

static Hephaestus::GpioPin ironStandPin(GPIOA, GPIO_PIN_4, true);
static Hephaestus::GpioPin airStandPin(GPIOA, GPIO_PIN_5, true);

static Hephaestus::FlashStorage storage;

static Hephaestus::StationManager station(
    storage, adc, ironPin, airPin, ironStandPin, airStandPin, ironEncoder, airEncoder, ironPwm, airPwm, airFanPwm, buzzerPwm
);

// RTOS Tasks
void appLoopTask(void*) {
    for(;;) {
        station.tickInput(HAL_GetTick());
        vTaskDelay(pdMS_TO_TICKS(15));
    }
}

void safetyTask(void*) {
    for(;;) {
        HAL_IWDG_Refresh(&hiwdg);

        if (station.hasSystemError()) {
            Hephaestus::Logger::fatal("SAFETY", "CRITICAL FAULT! KILLING POWER!");
            
            // Апаратно глушимо всі ШІМ-канали
            ironPwm.enable(false);
            airPwm.enable(false);
            airFanPwm.enable(false);
            
            // Вмикаємо зумер на постійний сигнал тривоги
            buzzerPwm.setDutyCycle(50.0f);
            buzzerPwm.enable(true);

            // ВБИВАЄМО ЖИВЛЕННЯ!
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET); 

            while(1) {
                vTaskDelay(pdMS_TO_TICKS(1000)); 
            }
        }

        // Перевіряємо стан кожні 20 мс (50 Гц)
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
void displayTask(void*) {
    for(;;) {
        station.tickDisplay();
        vTaskDelay(pdMS_TO_TICKS(80)); 
    }
}

void controlLoopTask(void*) {
    for(;;) {
        station.tickControl(HAL_GetTick());
        airPwm.tick(HAL_GetTick());
        
#ifdef PC_SIMULATOR
        mockAdc.applyHeat(ironPwm.getDutyCycle(), airPwm.getDutyCycle(), 0.05f);
#endif
        vTaskDelay(pdMS_TO_TICKS(50)); // Цикл керування 20 Гц
    }
}

// System initialization entry point
extern "C" void app_setup() {
    Hephaestus::i2c1Mutex = xSemaphoreCreateMutex();
    
    Hephaestus::Logger::init();
    Hephaestus::Logger::addSink(&uartLogSink);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);

    ironPwm.enable(true);
    airPwm.enable(true);
    airFanPwm.enable(true);

    station.init();

    xTaskCreate(Hephaestus::Logger::taskLoop, "Logger",  configMINIMAL_STACK_SIZE, nullptr, 1, nullptr);
    xTaskCreate(displayTask,                  "Display", configMINIMAL_STACK_SIZE, nullptr, 2, nullptr);
    xTaskCreate(appLoopTask,                  "Input",   configMINIMAL_STACK_SIZE, nullptr, 3, nullptr);
    xTaskCreate(controlLoopTask,              "Control", configMINIMAL_STACK_SIZE, nullptr, 4, nullptr);
    xTaskCreate(safetyTask,                   "Safety",  configMINIMAL_STACK_SIZE, nullptr, 5, nullptr);
}

// Main loop stub (controlled by RTOS scheduler)
extern "C" void app_loop() {
    vTaskDelay(1000);
}
