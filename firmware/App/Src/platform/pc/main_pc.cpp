// App/Src/platform/pc/main_pc.cpp
#include <iostream>
#include <cstdio>

#include "FreeRTOS.h"
#include "task.h"

#include "interfaces/IAdc.hpp"
#include "interfaces/IPwm.hpp"
#include "interfaces/IDigitalPin.hpp"
#include "interfaces/IEncoder.hpp"
#include "interfaces/ILogSink.hpp"

#include "logic/station_manager.hpp"
#include "system/logger/logger.hpp"

extern "C" uint32_t HAL_GetTick(); 

namespace Hephaestus {

    // =========================================================================
    // HARDWARE MOCKS FOR PC SIMULATION
    // =========================================================================
    
    class MockAdc : public IAdc {
        float readVoltage(uint8_t channel) override {
            switch (channel) {
                case 0: return 2.18f; // PSU (~24V)
                case 1: return 1.65f; // NTC (~25C)
                case 2: return 0.05f; // Iron TC
                case 3: return 0.05f; // Air TC
                default: return 0.0f;
            }
        }
    };

    class MockPwm : public IPwm {
        void setDutyCycle(float percent) override {} 
        void enable(bool state) override {}
    };

    class MockPin : public IDigitalPin {
        bool isActive() override { return false; } 
    };

    class MockEncoder : public IEncoder {
        void init() override {}
        EncoderResult getSteps() override { return {0, 0}; }
    };

    class ConsoleLogSink : public ILogSink {
        bool isReady() const override { return true; }
        void write(const uint8_t* data, size_t length) override {
            fwrite(data, 1, length, stdout);
            fflush(stdout); 
        }
    };

} // namespace Hephaestus

// =========================================================================
// SYSTEM INSTANTIATION
// =========================================================================

static Hephaestus::MockAdc     mockAdc;
static Hephaestus::MockPin     mockPin;
static Hephaestus::MockEncoder mockEnc;
static Hephaestus::MockPwm     mockPwm;
static Hephaestus::ConsoleLogSink consoleSink;

static Hephaestus::StationManager station(
    mockAdc, mockPin, mockPin, mockEnc, mockEnc, mockPwm, mockPwm
);

// =========================================================================
// FREERTOS TASKS (SIMULATED VIA PTHREADS)
// =========================================================================

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

// =========================================================================
// ENTRY POINT
// =========================================================================

int main() {
    std::cout << "Starting Hephaestus FreeRTOS Simulator on Linux...\n";

    Hephaestus::Logger::init();
    Hephaestus::Logger::addSink(&consoleSink);

    station.init();

    // Stack size is specified in bytes for the POSIX port (16KB per task)
    xTaskCreate(Hephaestus::Logger::taskLoop, "Logger",  16384, nullptr, 1, nullptr);
    xTaskCreate(displayTask,                  "Display", 16384, nullptr, 2, nullptr);
    xTaskCreate(appLoopTask,                  "Input",   16384, nullptr, 3, nullptr);

    Hephaestus::Logger::info("SYS", "Simulator RTOS Scheduler Starting...");

    vTaskStartScheduler();

    return 0;
}
