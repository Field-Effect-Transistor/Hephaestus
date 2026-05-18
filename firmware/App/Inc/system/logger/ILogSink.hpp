//  App/Inc/logger/ILogSink.hpp
#pragma once

#include <cstdint>
#include <cstdlib>

namespace Hephaestus {
    struct ILogSink {
    public:
        virtual ~ILogSink() = default;
        virtual bool isReady() const = 0;
        virtual void write(const uint8_t* data, size_t length) = 0;
    };  //  class   ILogSink
}   //  namespace   Hephaestus
