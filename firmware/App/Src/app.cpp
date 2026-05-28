// App/Src/app.cpp
#include "app.hpp"
#include "usart.h"
#include "gpio.h"
#include "tim.h"
#include "i2c.h"

// Інтерфейси та Бізнес-логіка
#include "system/logger/logger.hpp"
#include "system/logger/uart_log_sink.hpp"
#include "logic/button.hpp"
#include "system/encoder.hpp"
#include "logic/heater_channel.hpp"
#include "ui/display_manager.hpp"
#include "system/math_sensors.hpp"
#include "system/system_config.hpp"

// Платформозалежні драйвери (STM32)
#include "system/i2c_arbiter.hpp"
#include "platform/stm32/adc_ads1115.hpp"
#include "platform/stm32/pwm_driver_tim.hpp"

SemaphoreHandle_t Hephaestus::i2c1Mutex = nullptr;

// ---------------------------------------------------------
// 1. СТВОРЕННЯ АПАРАТНИХ ДРАЙВЕРІВ
// ---------------------------------------------------------
static Hephaestus::UARTLogSink uartLogSink(&huart1);
static Hephaestus::Ads1115 adc(&hi2c1);

static Hephaestus::Button ironBtn; 
static Hephaestus::Button airBtn;  
static Hephaestus::Encoder ironEncoder(&htim2); 
static Hephaestus::Encoder airEncoder(&htim4);  

// ШІМ Драйвери
static Hephaestus::PwmDriverTim ironPwm(&htim1, TIM_CHANNEL_1);
static Hephaestus::PwmDriverTim airPwm(&htim1, TIM_CHANNEL_4);

// ---------------------------------------------------------
// 2. СТВОРЕННЯ БІЗНЕС-ЛОГІКИ
// ---------------------------------------------------------
static Hephaestus::SystemConfig sysConfig;

static Hephaestus::HeaterChannel ironChannel("IRON", ironPwm, 300, 100, 450, 150);
static Hephaestus::HeaterChannel airChannel("AIR", airPwm, 300, 100, 500, 50);

static Hephaestus::DisplayManager display;

// ---------------------------------------------------------
// 3. ДОПОМІЖНІ ОБРОБНИКИ (Оголошуємо ЇХ ДО ВИКЛИКУ В APP_LOOP!)
// ---------------------------------------------------------
static void handleButton(Hephaestus::HeaterChannel& channel, Hephaestus::ButtonEvent event) {
    if (event == Hephaestus::ButtonEvent::SingleClick) {
        channel.toggleState(); 
    } 
    else if (event == Hephaestus::ButtonEvent::DoubleClick || event == Hephaestus::ButtonEvent::LongPress) {
        channel.setState(Hephaestus::ChannelState::Sleep); 
    }
}

static void handleEncoder(Hephaestus::HeaterChannel& channel, Hephaestus::EncoderResult enc, bool isPressed) {
    if (!enc.hasMovement()) return;

    if (isPressed) {
        int16_t direction = (enc.raw > 0) ? 1 : -1;
        channel.changeTargetTemp(direction * 50); // Push & Turn
    } else {
        channel.changeTargetTemp(enc.accelerated);
    }
}

// ---------------------------------------------------------
// 4. ЗАДАЧІ FREERTOS
// ---------------------------------------------------------
extern "C" void app_loop() {
    uint32_t tick = HAL_GetTick();

    // Читаємо кнопки
    bool isIronPressed = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET);
    bool isAirPressed  = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_RESET);

    Hephaestus::ButtonEvent ironEvent = ironBtn.update(isIronPressed, tick);
    Hephaestus::ButtonEvent airEvent  = airBtn.update(isAirPressed, tick);
    
    Hephaestus::EncoderResult ironEnc = ironEncoder.getSteps();
    Hephaestus::EncoderResult airEnc  = airEncoder.getSteps();

    // Обробляємо події (Ось тут тепер компілятор бачить ці функції)
    handleButton(ironChannel, ironEvent);
    handleButton(airChannel, airEvent);
    handleEncoder(ironChannel, ironEnc, isIronPressed);
    handleEncoder(airChannel, airEnc, isAirPressed);

    // Оновлюємо потужність ШІМ згідно зі станом каналу
    ironChannel.updateControlLoop();
    airChannel.updateControlLoop();

    // ДЛЯ ТЕСТУ UI: Показуємо TargetTemp як CurrentTemp на екрані
    ironChannel.setCurrentTemp(ironChannel.getTargetTemp());
    airChannel.setCurrentTemp(airChannel.getTargetTemp());

    vTaskDelay(pdMS_TO_TICKS(15));
}

void appLoopTask(void*) {
    for(;;) {
        app_loop();
    }
}

void displayTask(void* params) {
    display.init(); 
    Hephaestus::Logger::info("SYS", "Display initialized");

    for(;;) {
        display.update(ironChannel, airChannel);
        vTaskDelay(pdMS_TO_TICKS(80)); 
    }
}

// ---------------------------------------------------------
// 5. ІНІЦІАЛІЗАЦІЯ
// ---------------------------------------------------------
extern "C" void app_setup() {
    Hephaestus::i2c1Mutex = xSemaphoreCreateMutex();
    
    Hephaestus::Logger::init();
    Hephaestus::Logger::addSink(&uartLogSink);

    ironEncoder.init();
    airEncoder.init();
    
    // Вмикаємо ШІМ-таймери
    ironPwm.enable(true);
    airPwm.enable(true);

    xTaskCreate(Hephaestus::Logger::taskLoop, "LoggerTask", 256, nullptr, tskIDLE_PRIORITY + 1, nullptr);
    xTaskCreate(displayTask, "DisplayTask", 384, nullptr, tskIDLE_PRIORITY + 2, nullptr);
    xTaskCreate(appLoopTask, "AppLoopTask", 256, nullptr, tskIDLE_PRIORITY + 3, nullptr);

    Hephaestus::Logger::info("SYS", "Hephaestus UI Mode Booted!");
}
