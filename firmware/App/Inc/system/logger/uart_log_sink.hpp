// App/Inc/system/logger/uart_log_sink.hpp
#pragma once

#include "system/logger/ILogSink.hpp"
#include "usart.h"

namespace Hephaestus {
    class UARTLogSink : public ILogSink {
    private:
        UART_HandleTypeDef* _huart;

    public:
        explicit UARTLogSink(UART_HandleTypeDef* huart) : _huart(huart) {}

        bool isReady() const override { 
            return (_huart != nullptr) && (_huart->gState == HAL_UART_STATE_READY); 
        }

        void write(const uint8_t* data, size_t length) override {
            if (_huart == nullptr) return;

            HAL_UART_Transmit(_huart, data, length, HAL_MAX_DELAY);
        }
    };  //  class UARTLogSink
}  //   namespace Hephaestus