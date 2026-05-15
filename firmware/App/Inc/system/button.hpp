//  App/Inc/system/button.hpp
#pragma once

#include "FreeRTOS.h"
#include "queue.h"

//#include "exti_registry.hpp"

namespace Hephaestus {
    struct ButtonEvent {
        uint32_t    timestamp_ms;
    };

    class Button {
    private:
        uint16_t        _pin;
        QueueHandle_t   _queue;

        static void on_exti(void* constext);

    public:
        QueueHandle_t get_queue() const { return _queue; }

        Button(uint16_t pin, uint8_t queue_depth = 4);
        ~Button();

        Button(Button&&) = delete;
        Button(const Button&) = delete;

        Button& operator=(const Button&) = delete;
        Button& operator=(Button&&) = delete;

    };  //  class Button
}   //  namespace Hephaesus
