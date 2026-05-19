//  App/Src/system/logger/uart_log_sink.cpp
#include "system/logger/uart_log_sink.hpp"

#include "usart.h"

namespace Hephaestus {
    bool UARTLogSink::isReady() const { 
        return (_huart != nullptr) && (_huart->gState == HAL_UART_STATE_READY); 
    }

    void UARTLogSink::write(const uint8_t* data, size_t length) {
        if (_huart == nullptr) return;

        HAL_UART_Transmit(_huart, data, length, HAL_MAX_DELAY);
    }

}   //  namespace Hephaestus
