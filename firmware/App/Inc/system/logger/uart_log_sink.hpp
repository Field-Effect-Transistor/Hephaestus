// App/Inc/system/logger/uart_log_sink.hpp
#pragma once

#include "interfaces/ILogSink.hpp"

struct __UART_HandleTypeDef;
typedef __UART_HandleTypeDef UART_HandleTypeDef;

namespace Hephaestus {
    class UARTLogSink : public ILogSink {
    private:
        UART_HandleTypeDef* _huart;

    public:
        explicit UARTLogSink(UART_HandleTypeDef* huart) : _huart(huart) {}

        bool isReady() const override;
        void write(const uint8_t* data, size_t length) override;
    };  //  class UARTLogSink
}  //   namespace Hephaestus
