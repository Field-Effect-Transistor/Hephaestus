// App/Src/app.cpp
#include "app.hpp"
#include "usart.h"
#include "gpio.h"
#include "tim.h"
#include "i2c.h"

#include "system/logger/logger.hpp"
#include "system/logger/uart_log_sink.hpp"
#include "system/button.hpp"
#include "system/encoder.hpp"
#include "system/heater_channel.hpp"
#include "system/display_manager.hpp"

// ---------------------------------------------------------
static Hephaestus::UARTLogSink uartLogSink(&huart1);

static Hephaestus::Button ironBtn; 
static Hephaestus::Button airBtn;  

static Hephaestus::Encoder ironEncoder(&htim2); 
static Hephaestus::Encoder airEncoder(&htim4);  

// Ініціалізуємо канали: Назва, Задана(default), Мін, Макс, Сон
static Hephaestus::HeaterChannel ironChannel("IRON", 300, 100, 450, 150);
static Hephaestus::HeaterChannel airChannel("AIR", 300, 100, 500, 50);

static Hephaestus::DisplayManager display;

// ---------------------------------------------------------
// ЗАДАЧІ FREERTOS
// ---------------------------------------------------------
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
        vTaskDelay(pdMS_TO_TICKS(80)); // Оновлення ~12 FPS
    }
}

extern "C" void app_setup() {
    Hephaestus::Logger::init();
    Hephaestus::Logger::addSink(&uartLogSink);

    ironEncoder.init();
    airEncoder.init();

    // Створюємо задачі з правильними пріоритетами
    xTaskCreate(Hephaestus::Logger::taskLoop, "LoggerTask", 256, nullptr, tskIDLE_PRIORITY + 1, nullptr);
    xTaskCreate(displayTask, "DisplayTask", 384, nullptr, tskIDLE_PRIORITY + 2, nullptr);
    xTaskCreate(appLoopTask, "AppLoopTask", 256, nullptr, tskIDLE_PRIORITY + 3, nullptr);

    Hephaestus::Logger::info("SYS", "Hephaestus UI Mode Booted!");
}

// ---------------------------------------------------------
// ОБРОБКА ВВОДУ
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
        channel.changeTargetTemp(direction * 50); // Push & Turn: крок 50 градусів
    } else {
        channel.changeTargetTemp(enc.accelerated); // Звичайне обертання
    }
}

// ---------------------------------------------------------
// ГОЛОВНИЙ ЦИКЛ (Викликається з AppLoopTask кожні 15мс)
// ---------------------------------------------------------
extern "C" void app_loop() {
    uint32_t tick = HAL_GetTick();

    // Читаємо піни кнопок (натиснення = LOW)
    bool isIronPressed = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET);
    bool isAirPressed  = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_RESET);

    // Оновлюємо автомати станів кнопок та отримуємо кроки енкодерів
    Hephaestus::ButtonEvent ironEvent = ironBtn.update(isIronPressed, tick);
    Hephaestus::ButtonEvent airEvent  = airBtn.update(isAirPressed, tick);
    
    Hephaestus::EncoderResult ironEnc = ironEncoder.getSteps();
    Hephaestus::EncoderResult airEnc  = airEncoder.getSteps();

    // Передаємо події в канали нагрівачів
    handleButton(ironChannel, ironEvent);
    handleButton(airChannel, airEvent);
    
    handleEncoder(ironChannel, ironEnc, isIronPressed);
    handleEncoder(airChannel, airEnc, isAirPressed);

    // ДЛЯ ТЕСТУ UI: Відображаємо цільову температуру як поточну
    ironChannel.setCurrentTemp(ironChannel.getTargetTemp());
    airChannel.setCurrentTemp(airChannel.getTargetTemp());

    // Антибрязкіт і звільнення процесора
    vTaskDelay(pdMS_TO_TICKS(15));
}