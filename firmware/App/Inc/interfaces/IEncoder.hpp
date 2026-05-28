//  App/Inc/interfaces/IEncoder.hpp
#pragma once
#include <cstdint>

namespace Hephaestus {
    struct EncoderResult {
        int16_t raw;
        int16_t accelerated;
        bool hasMovement() const { return raw != 0; }
    };

    class IEncoder {
    public:
        virtual ~IEncoder() = default;
        virtual void init() = 0;
        virtual EncoderResult getSteps() = 0;
    };
}
