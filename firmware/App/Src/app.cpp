// App/Src/app.cpp
#include "app.hpp"
#include "usart.h"
#include "gpio.h"

#include "system/logger/logger.hpp"
#include "system/logger/uart_log_sink.hpp"
#include "system/button.hpp"

static Hephaestus::UARTLogSink uartLogSink(&huart1);

static Hephaestus::Button ironBtn; // PB12
static Hephaestus::Button airBtn;  // PB13

void appLoopTask(void*) {
    for(;;) {
        app_loop();
    }
}

extern "C" void app_setup() {
    Hephaestus::Logger::init();
    Hephaestus::Logger::addSink(&uartLogSink);

    xTaskCreate(
        Hephaestus::Logger::taskLoop, 
        "LoggerTask", 
        256,
        nullptr,
        tskIDLE_PRIORITY + 1,
        nullptr
    );

    xTaskCreate(
        appLoopTask,
        "app loop task",
        256,
        nullptr,
        tskIDLE_PRIORITY,
        nullptr
    );

    Hephaestus::Logger::info("SYS", "Hephaestus Soldering Station Booted!");
    Hephaestus::Logger::debug("SYS", "SystemCoreClock: %lu MHz", SystemCoreClock / 1000000);
}

static void logButtonEvent(const char* btnName, Hephaestus::ButtonEvent event) {
    using namespace Hephaestus;
    switch (event) {
        case ButtonEvent::SingleClick:
            Logger::info("INPUT", "[%s] SINGLE CLICK", btnName);
            break;
        case ButtonEvent::DoubleClick:
            Logger::info("INPUT", "[%s] DOUBLE CLICK", btnName);
            break;
        case ButtonEvent::LongPress:
            Logger::info("INPUT", "[%s] HOLD START", btnName);
            break;
        case ButtonEvent::LongPressRepeat:
            Logger::debug("INPUT", "[%s] Holding...", btnName);
            break;
        default:
            break;
    }
}

extern "C" void app_loop() {
    uint32_t tick = HAL_GetTick();

    bool isIronPressed = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET);
    bool isAirPressed  = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_RESET);

    Hephaestus::ButtonEvent ironEvent = ironBtn.update(isIronPressed, tick);
    Hephaestus::ButtonEvent airEvent  = airBtn.update(isAirPressed, tick);

    logButtonEvent("IRON", ironEvent);
    logButtonEvent("AIR ", airEvent);

    vTaskDelay(pdMS_TO_TICKS(15));
}
