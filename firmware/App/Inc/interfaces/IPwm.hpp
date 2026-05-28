//  App/Inc/interfaces/IPwm.hpp
#pragma once

namespace Hephaestus {
    class IPwm {
    public:
        virtual ~IPwm() = default;
        virtual void setDutyCycle(float percent) = 0; 
        virtual void enable(bool state) = 0;
        virtual float getDutyCycle() const = 0;
    };
}
