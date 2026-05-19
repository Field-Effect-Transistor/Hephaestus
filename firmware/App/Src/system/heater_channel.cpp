// App/Src/system/heater_channel.cpp
#include "system/heater_channel.hpp"
#include "system/logger/logger.hpp"

namespace Hephaestus {

    HeaterChannel::HeaterChannel(const char* name, int16_t defaultTemp, int16_t minTemp, int16_t maxTemp, int16_t sleepTemp)
        : _name(name), 
          _targetTemp(defaultTemp), 
          _currentTemp(0), 
          _pwmDuty(0), 
          _state(ChannelState::Off),
          _minTemp(minTemp), 
          _maxTemp(maxTemp), 
          _sleepTemp(sleepTemp) 
    {
        // Базова перевірка, щоб defaultTemp був у межах
        if (_targetTemp < _minTemp) _targetTemp = _minTemp;
        if (_targetTemp > _maxTemp) _targetTemp = _maxTemp;
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
        if (_state == newState) return; // Нічого не змінилося

        _state = newState;

        // Логуємо зміну стану
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

        // Clamping (Захист меж)
        _targetTemp = newTemp;
        if (_targetTemp < _minTemp) _targetTemp = _minTemp;
        if (_targetTemp > _maxTemp) _targetTemp = _maxTemp;

        // Логуємо тільки якщо значення реально змінилося
        if (oldTemp != _targetTemp) {
            Logger::info("LOGIC", "[%s] Target Temp: %d C", _name, _targetTemp);
        }
    }

} // namespace Hephaestus
