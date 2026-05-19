// App/Src/app.cpp
#include "app.hpp"
#include "usart.h"
#include "gpio.h"
#include "tim.h"

#include "system/logger/logger.hpp"
#include "system/logger/uart_log_sink.hpp"
#include "system/button.hpp"
#include "system/encoder.hpp"
#include "system/heater_channel.hpp"
#include "system/display_manager.hpp"

static Hephaestus::UARTLogSink uartLogSink(&huart1);

static Hephaestus::Button ironBtn; // PB12
static Hephaestus::Button airBtn;  // PB13

static Hephaestus::Encoder ironEncoder(&htim2); // PA0, PA1
static Hephaestus::Encoder airEncoder(&htim4);  // PB6, PB7

static Hephaestus::HeaterChannel ironChannel("IRON", 300, 100, 450, 150);
static Hephaestus::HeaterChannel airChannel("AIR", 300, 100, 500, 50);

static Hephaestus::DisplayManager display;

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

extern "C" void app_setup() {
    Hephaestus::Logger::init();
    Hephaestus::Logger::addSink(&uartLogSink);

    ironEncoder.init();
    airEncoder.init();

    xTaskCreate(Hephaestus::Logger::taskLoop, "LoggerTask", 256, nullptr, tskIDLE_PRIORITY + 1, nullptr);

    xTaskCreate(displayTask, "DisplayTask", 384, nullptr, tskIDLE_PRIORITY + 2, nullptr);

    xTaskCreate(appLoopTask, "app loop task", 256, nullptr, tskIDLE_PRIORITY + 3, nullptr);

    Hephaestus::Logger::info("SYS", "Hephaestus Soldering Station Booted!");
}

static void handleButton(Hephaestus::HeaterChannel& channel, Hephaestus::ButtonEvent event) {
    using namespace Hephaestus;
    if (event == ButtonEvent::SingleClick) {
        channel.toggleState(); // Увімкнути/Вимкнути
    } 
    else if (event == ButtonEvent::DoubleClick || event == ButtonEvent::LongPress) {
        channel.setState(ChannelState::Sleep); // Відправити спати
    }
}

static void handleEncoder(Hephaestus::HeaterChannel& channel, Hephaestus::EncoderResult enc, bool isPressed) {
    if (!enc.hasMovement()) return;

    if (isPressed) {
        int16_t direction = (enc.raw > 0) ? 1 : -1;
        channel.changeTargetTemp(direction * 50);
    } else {
        channel.changeTargetTemp(enc.accelerated);
    }
}

extern "C" void app_loop() {
    uint32_t tick = HAL_GetTick();

    bool isIronPressed = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET);
    bool isAirPressed  = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_RESET);

    Hephaestus::ButtonEvent ironEvent = ironBtn.update(isIronPressed, tick);
    Hephaestus::ButtonEvent airEvent  = airBtn.update(isAirPressed, tick);
    Hephaestus::EncoderResult ironEnc = ironEncoder.getSteps();
    Hephaestus::EncoderResult airEnc  = airEncoder.getSteps();

    handleButton(ironChannel, ironEvent);
    handleButton(airChannel, airEvent);

    handleEncoder(ironChannel, ironEnc, isIronPressed);
    handleEncoder(airChannel, airEnc, isAirPressed);

    vTaskDelay(pdMS_TO_TICKS(15));
}
