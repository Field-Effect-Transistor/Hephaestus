#include <iostream>
#include <cstdio>
#include <SDL2/SDL.h>
#include <pthread.h> // Для pthread_create

#include "FreeRTOS.h"
#include "task.h"

#include "platform/pc/sdl_context.hpp"
#include "interfaces/IAdc.hpp"
#include "interfaces/IPwm.hpp"
#include "interfaces/IDigitalPin.hpp"
#include "interfaces/IEncoder.hpp"
#include "interfaces/ILogSink.hpp"

#include "logic/station_manager.hpp"
#include "system/logger/logger.hpp"

extern "C" uint32_t HAL_GetTick(); 

// Єдиний глобальний контекст симулятора
Hephaestus::SdlContext sdlContext;

namespace Hephaestus {

    // --- Mocks ---
    class SdlPinIron : public IDigitalPin {
        bool isActive() override { return sdlContext.isIronPressed(); } 
    };
    class SdlPinAir : public IDigitalPin {
        bool isActive() override { return sdlContext.isAirPressed(); } 
    };
    class SdlEncoderIron : public IEncoder {
        void init() override {}
        EncoderResult getSteps() override { 
            int16_t diff = sdlContext.getIronEncDiff();
            return {diff, diff}; 
        }
    };
    class SdlEncoderAir : public IEncoder {
        void init() override {}
        EncoderResult getSteps() override { 
            int16_t diff = sdlContext.getAirEncDiff();
            return {diff, diff}; 
        }
    };
    class MockAdc : public IAdc {
        float readVoltage(uint8_t ch) override { return (ch == 0) ? 24.0f : 0.05f; }
    };
    class MockPwm : public IPwm {
        void setDutyCycle(float p) override {} 
        void enable(bool s) override {}
    };
    class ConsoleLogSink : public ILogSink {
        bool isReady() const override { return true; }
        void write(const uint8_t* d, size_t l) override { fwrite(d, 1, l, stdout); fflush(stdout); }
    };
}

// --- ІНСТАНЦІЮВАННЯ СИСТЕМИ ---
static Hephaestus::MockAdc        mockAdc;
static Hephaestus::SdlPinIron     pinIron;
static Hephaestus::SdlPinAir      pinAir;
static Hephaestus::SdlEncoderIron encIron;
static Hephaestus::SdlEncoderAir  encAir;
static Hephaestus::MockPwm        mockPwm;
static Hephaestus::ConsoleLogSink consoleSink;

static Hephaestus::StationManager station(
    mockAdc, pinIron, pinAir, encIron, encAir, mockPwm, mockPwm
);

// --- ЗАДАЧІ FREERTOS ---
void appLoopTask(void*) {
    for(;;) {
        station.tickInput(HAL_GetTick());
        vTaskDelay(pdMS_TO_TICKS(15));
    }
}

void displayTask(void*) {
    station.initDisplay();
    for(;;) {
        station.tickDisplay();
        vTaskDelay(pdMS_TO_TICKS(80)); 
    }
}

// Функція для запуску FreeRTOS у фоні
void* rtosThreadRunner(void* arg) {
    std::cout << "Starting FreeRTOS Scheduler in Background Thread...\n";
    vTaskStartScheduler();
    return nullptr;
}

// --- ENTRY POINT ---
int main() {
    std::cout << "Starting Hephaestus FreeRTOS Simulator...\n";
    std::cout << "[UP/DOWN, ENTER] - Iron | [W/S, SPACE] - Air\n\n";

    Hephaestus::Logger::init();
    Hephaestus::Logger::addSink(&consoleSink);

    // SDL ініціалізується ДО старту RTOS-потоку
    sdlContext.init();

    station.init();

    xTaskCreate(Hephaestus::Logger::taskLoop, "Logger",  16384, nullptr, 1, nullptr);
    xTaskCreate(displayTask,                  "Display", 16384, nullptr, 2, nullptr);
    xTaskCreate(appLoopTask,                  "Input",   16384, nullptr, 3, nullptr);

    pthread_t rtosThread;
    pthread_create(&rtosThread, nullptr, rtosThreadRunner, nullptr);

    // Main thread повністю під SDL — poll events + render
    sdlContext.runLoop(); // блокує назавжди

    return 0;
}
