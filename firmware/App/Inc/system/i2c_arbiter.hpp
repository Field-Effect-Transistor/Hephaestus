//  App/Inc/system/i2c_arbiter.hpp
#pragma once
#include "FreeRTOS.h"
#include "semphr.h"

namespace Hephaestus {
    extern SemaphoreHandle_t i2c1Mutex;
}
