// App/Src/platform/pc/main_pc.cpp
#include <iostream>
#include <cstdio>
#include <SDL2/SDL.h>
#include <pthread.h> 

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

Hephaestus::SdlContext sdlContext;

namespace Hephaestus {

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
    public:
        float simulatedTempIron = 25.0f;
        float simulatedTempAir  = 25.0f;

        float readVoltage(uint8_t channel) override {
            if (channel == 2) { // IRON TC (Gain 201)
                return (simulatedTempIron - 25.0f) * 0.041f * 201.0f / 1000.0f;
            }
            if (channel == 3) { // AIR TC (Gain 101)
                return (simulatedTempAir - 25.0f) * 0.041f * 101.0f / 1000.0f;
            }
            if (channel == 0) return 2.18f; // PSU
            if (channel == 1) return 1.65f; // NTC (25C)
            
            return 0.0f;
        }

        void applyHeat(float ironDuty, float airDuty, float dt) {
            simulatedTempIron += (ironDuty * 1.5f * dt) - ((simulatedTempIron - 25.0f) * 0.1f * dt);
            if (simulatedTempIron < 25.0f) simulatedTempIron = 25.0f;

            simulatedTempAir += (airDuty * 2.5f * dt) - ((simulatedTempAir - 25.0f) * 0.15f * dt);
            if (simulatedTempAir < 25.0f) simulatedTempAir = 25.0f;
        }
    };

    class MockPwm : public IPwm {
    public:
        float currentDuty = 0.0f;
        void setDutyCycle(float p) override { currentDuty = p; } 
        void enable(bool s) override { if(!s) currentDuty = 0.0f; }
        float getDutyCycle() const { return currentDuty; }
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
static Hephaestus::MockPwm        ironPwm; // Окремо для паяльника
static Hephaestus::MockPwm        airPwm;  // Окремо для фена
static Hephaestus::ConsoleLogSink consoleSink;

static Hephaestus::StationManager station(
    mockAdc, pinIron, pinAir, encIron, encAir, ironPwm, airPwm
);

// --- ЗАДАЧІ FREERTOS ---
void appLoopTask(void*) {
    for(;;) {
        station.tickInput(HAL_GetTick());
        
        // Гріємо віртуальні паяльник і фен!
        mockAdc.applyHeat(ironPwm.getDutyCycle(), airPwm.getDutyCycle(), 0.015f);
        
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

void* rtosThreadRunner(void* arg) {
    std::cout << "Starting FreeRTOS Scheduler in Background Thread...\n";
    vTaskStartScheduler();
    return nullptr;
}

int main() {
    std::cout << "Starting Hephaestus FreeRTOS Simulator...\n";
    std::cout << "[UP/DOWN, ENTER] - Iron | [W/S, SPACE] - Air\n\n";

    Hephaestus::Logger::init();
    Hephaestus::Logger::addSink(&consoleSink);

    sdlContext.init();
    station.init();

    xTaskCreate(Hephaestus::Logger::taskLoop, "Logger",  16384, nullptr, 1, nullptr);
    xTaskCreate(displayTask,                  "Display", 16384, nullptr, 2, nullptr);
    xTaskCreate(appLoopTask,                  "Input",   16384, nullptr, 3, nullptr);

    pthread_t rtosThread;
    pthread_create(&rtosThread, nullptr, rtosThreadRunner, nullptr);

    sdlContext.runLoop(); 

    return 0;
}