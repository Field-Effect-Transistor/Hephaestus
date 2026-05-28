#include "app.hpp"
#include "usart.h"
#include "tim.h"
#include "i2c.h"

#include "system/logger/logger.hpp"
#include "system/logger/uart_log_sink.hpp"
#include "system/i2c_arbiter.hpp"

#include "platform/stm32/adc_ads1115.hpp"
#include "platform/stm32/pwm_driver_tim.hpp"
#include "platform/stm32/gpio_pin.hpp"
#include "platform/stm32/encoder_tim.hpp"

#include "logic/station_manager.hpp"

SemaphoreHandle_t Hephaestus::i2c1Mutex = nullptr;

// Hardware drivers instantiation
static Hephaestus::UARTLogSink uartLogSink(&huart1);
static Hephaestus::Ads1115     adc(&hi2c1);

static Hephaestus::GpioPin ironPin(GPIOB, GPIO_PIN_12);
static Hephaestus::GpioPin airPin(GPIOB, GPIO_PIN_13);

static Hephaestus::EncoderTim ironEncoder(&htim2);
static Hephaestus::EncoderTim airEncoder(&htim4);

static Hephaestus::PwmDriverTim ironPwm(&htim1, TIM_CHANNEL_1);
static Hephaestus::PwmDriverTim airPwm(&htim1, TIM_CHANNEL_4);

// System controller instantiation
static Hephaestus::StationManager station(
    adc, ironPin, airPin, ironEncoder, airEncoder, ironPwm, airPwm
);

// RTOS Tasks
void appLoopTask(void*) {
    for(;;) {
        station.tickInput(HAL_GetTick());
        vTaskDelay(pdMS_TO_TICKS(15));
    }
}

void displayTask(void*) {
    for(;;) {
        station.tickDisplay();
        vTaskDelay(pdMS_TO_TICKS(80)); 
    }
}

// System initialization entry point
extern "C" void app_setup() {
    Hephaestus::i2c1Mutex = xSemaphoreCreateMutex();
    
    Hephaestus::Logger::init();
    Hephaestus::Logger::addSink(&uartLogSink);

    ironPwm.enable(true);
    airPwm.enable(true);

    station.init();

    xTaskCreate(Hephaestus::Logger::taskLoop, "Logger", 256, nullptr, tskIDLE_PRIORITY + 1, nullptr);
    xTaskCreate(displayTask, "Display", 384, nullptr, tskIDLE_PRIORITY + 2, nullptr);
    xTaskCreate(appLoopTask, "Input", 256, nullptr, tskIDLE_PRIORITY + 3, nullptr);
}

// Main loop stub (controlled by RTOS scheduler)
extern "C" void app_loop() {
    vTaskDelay(1000);
}
