//  App/Inc/system/exti_registry.hpp
#pragma once

#include "main.h"
#include <cstdint>

#include "stm32f103xb.h"

namespace Hephaestus {
    class EXTIRegistry {
    public:
        typedef void(*Callback_t)(void* context);
        struct Handler_t {
            Callback_t  callback = nullptr;
            void* context = nullptr;

            Handler_t(): callback(nullptr), context(nullptr) {}
        };

        EXTIRegistry() = delete;
        EXTIRegistry(const EXTIRegistry&) = delete;
        EXTIRegistry(EXTIRegistry&&) = delete;

        EXTIRegistry& operator=(const EXTIRegistry&) = delete;
        EXTIRegistry& operator=(EXTIRegistry&&) = delete;

        ~EXTIRegistry() = default;

    private:
        inline static Handler_t     _handlers[16];
        inline static const uint8_t _pin_to_index(const uint16_t pin) {
            if (pin == 0) return 0xFF;
            return static_cast<uint8_t>(__builtin_ctz(pin));
        }  

    public:
        inline static void set_handler(uint16_t pin, const Handler_t& handler) {
            auto index = _pin_to_index(pin);
            if (index < 16) {
                uint32_t primask = __get_PRIMASK();
                __disable_irq();

                _handlers[index] = handler;

                __set_PRIMASK(primask);
            }
        }

        inline static void set_handler(uint16_t pin, const Callback_t callback, void* context = nullptr) {
            auto index = _pin_to_index(pin);
            if (index < 16) {
                auto handler = _handlers + index;

                uint32_t primask = __get_PRIMASK();
                __disable_irq();

                handler->callback = callback;
                handler->context  = context;

                __set_PRIMASK(primask);
            }
        }

        inline static void remove_handler(uint16_t pin) {
            set_handler(pin, nullptr, nullptr); //  the greatest exsmple of code reuse)
        }

        inline static void dispatch(uint16_t pin) {
            auto index = _pin_to_index(pin);
            if (index < 16) {
                auto handler = _handlers + index;
                if (handler->callback) {
                    handler->callback(handler->context);
                }
            }
        }

    };  //  class EXTIRegistry
}   //  namespace Hephaestus
