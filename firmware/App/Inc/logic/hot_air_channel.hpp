#pragma once

#include "logic/heater_channel.hpp"

namespace Hephaestus {

    class HotAirChannel : public HeaterChannel {
    private:
        IPwm&   _fanPwmDriver;
        float   _fanDuty;          
        int16_t _safeCoolingTemp;  

    public:
        HotAirChannel(
            const char* name, 
            IPwm& heaterPwm, 
            IPwm& fanPwm, 
            int16_t defaultTemp, 
            int16_t minTemp, 
            int16_t maxTemp, 
            int16_t safeCoolingTemp,
            float kp, float ki, float kd,
            uint16_t sleepTimeoutSec
        );

        void setFanSpeed(float percent);
        float getFanSpeed() const { return _fanDuty; }

        void updateControlLoop(float dt) override;
    };

} // namespace Hephaestus