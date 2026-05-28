//  App/Inc/Interfaces.hpp
#pragma once

namespace Hephaestus {
    class IDigitalPin {
    public:
        virtual ~IDigitalPin() = default;
        virtual bool isActive() = 0; 
    };
}
