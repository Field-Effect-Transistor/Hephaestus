// App/Src/logic/hot_air_channel.cpp
#include "logic/hot_air_channel.hpp"
#include "system/logger/logger.hpp"

namespace Hephaestus {

    HotAirChannel::HotAirChannel(
        const char* name, IPwm& heaterPwm, IPwm& fanPwm, 
        int16_t defaultTemp, int16_t minTemp, int16_t maxTemp, int16_t safeCoolingTemp,
        float kp, float ki, float kd,
        uint16_t sleepTimeoutSec
    ) : HeaterChannel(name, heaterPwm, defaultTemp, minTemp, maxTemp, safeCoolingTemp, kp, ki, kd, sleepTimeoutSec),
        _fanPwmDriver(fanPwm), 
        _fanDuty(50.0f),
        _safeCoolingTemp(safeCoolingTemp)
    {
        _fanPwmDriver.setDutyCycle(0.0f);
    }

    void HotAirChannel::setFanSpeed(float percent) {
        if (percent < 15.0f) percent = 15.0f;
        if (percent > 100.0f) percent = 100.0f;
        _fanDuty = percent;
    }

    void HotAirChannel::updateControlLoop(float dt) {
        ChannelState currentState = getState();
        int16_t currentTemp = getCurrentTemp();

        if (currentState == ChannelState::Active) {
            HeaterChannel::updateControlLoop(dt);
            
            _fanPwmDriver.setDutyCycle(_fanDuty); 
        } 
        else {
            forcePwmOff(); 

            if (currentTemp > _safeCoolingTemp) {
                _fanPwmDriver.setDutyCycle(100.0f); 
            } else {
                _fanPwmDriver.setDutyCycle(0.0f);   
            }
        }
    }

} // namespace Hephaestus
