// App/Inc/system/IAdc.hpp
#pragma once
#include <cstdint>

namespace Hephaestus {
    class IAdc {
    public:
        virtual ~IAdc() = default;

        virtual float readVoltage(uint8_t channel) = 0;
    };
}