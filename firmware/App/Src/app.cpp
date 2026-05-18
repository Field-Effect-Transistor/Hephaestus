// App/Src/app.cpp
#include "app.hpp"
#include "usart.h"
#include "system/logger/logger.hpp"
#include "system/logger/uart_log_sink.hpp"

static Hephaestus::UARTLogSink uartLogSink(&huart1);

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

extern "C" void app_loop() {
    Hephaestus::Logger::debug("LOOP", "Ping from IDLE/Main thread");
    vTaskDelay(pdMS_TO_TICKS(1000));
}
