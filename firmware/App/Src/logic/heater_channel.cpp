// App/Src/logic/heater_channel.cpp
#include "logic/heater_channel.hpp"
#include "system/logger/logger.hpp"

namespace Hephaestus {

    HeaterChannel::HeaterChannel(const char* name, IPwm& pwmDriver, int16_t defaultTemp, int16_t minTemp, int16_t maxTemp, int16_t sleepTemp)
        : _name(name), 
          _pwmDriver(pwmDriver), 
          _targetTemp(defaultTemp), 
          _currentTemp(0), 
          _pwmDuty(0.0f), 
          _state(ChannelState::Off),
          _minTemp(minTemp), 
          _maxTemp(maxTemp), 
          _sleepTemp(sleepTemp) 
    {
        if (_targetTemp < _minTemp) _targetTemp = _minTemp;
        if (_targetTemp > _maxTemp) _targetTemp = _maxTemp;
        
        _pwmDriver.setDutyCycle(0.0f);
    }

    void HeaterChannel::toggleState() {
        if (_state == ChannelState::Error) {
            Logger::warn("LOGIC", "[%s] Cannot toggle: Error State! Reset required.", _name);
            return;
        }

        if (_state == ChannelState::Off || _state == ChannelState::Sleep) {
            setState(ChannelState::Active);
        } else {
            setState(ChannelState::Off);
        }
    }

    void HeaterChannel::setState(ChannelState newState) {
        if (_state == newState) return;

        _state = newState;

        const char* stateStr = "UNKNOWN";
        switch (_state) {
            case ChannelState::Off:    stateStr = "OFF"; break;
            case ChannelState::Active: stateStr = "ACTIVE"; break;
            case ChannelState::Sleep:  stateStr = "SLEEP"; break;
            case ChannelState::Error:  stateStr = "ERROR"; break;
        }

        Logger::info("LOGIC", "[%s] State changed to -> %s", _name, stateStr);
    }

    void HeaterChannel::changeTargetTemp(int16_t delta) {
        setTargetTemp(_targetTemp + delta);
    }

    void HeaterChannel::setTargetTemp(int16_t newTemp) {
        int16_t oldTemp = _targetTemp;

        _targetTemp = newTemp;
        if (_targetTemp < _minTemp) _targetTemp = _minTemp;
        if (_targetTemp > _maxTemp) _targetTemp = _maxTemp;

        if (oldTemp != _targetTemp) {
            Logger::info("LOGIC", "[%s] Target Temp: %d C", _name, _targetTemp);
        }
    }

    void HeaterChannel::updateControlLoop() {
        if (_state == ChannelState::Off || _state == ChannelState::Error) {
            _pwmDuty = 0.0f;
            _pwmDriver.setDutyCycle(_pwmDuty);
            return;
        }

        int16_t activeTarget = (_state == ChannelState::Sleep) ? _sleepTemp : _targetTemp;

        if (_currentTemp < activeTarget) {
            _pwmDuty = 100.0f;
        } else {
            _pwmDuty = 0.0f;
        }

        _pwmDriver.setDutyCycle(_pwmDuty);
    }

} // namespace Hephaestus