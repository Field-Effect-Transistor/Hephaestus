// App/Src/logic/heater_channel.cpp
#include "logic/heater_channel.hpp"
#include "system/logger/logger.hpp"

namespace Hephaestus {

    HeaterChannel::HeaterChannel(
        const char* name,
        IPwm& pwmDriver,
        int16_t defaultTemp,
        int16_t minTemp,
        int16_t maxTemp,
        int16_t sleepTemp,
        float kp,
        float ki,
        float kd,
        uint16_t sleepTimeoutSec 
    ) : _name(name), 
        _pwmDriver(pwmDriver), 
        _targetTemp(defaultTemp), 
        _currentTemp(0), 
        _pwmDuty(0.0f), 
        _state(ChannelState::Off),
        _minTemp(minTemp), 
        _maxTemp(maxTemp), 
        _sleepTemp(sleepTemp),
        _sleepTimeoutSec(sleepTimeoutSec),
        _idleTimeSec(0.0f),    
        _watchdogTimerSec(0.0f),
        _watchdogCheckpointTemp(0),
        _pid(kp, ki, kd, 0.0f, 100.0f)
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
        
        if (newState == ChannelState::Active) {
            _pid.reset(); 
            _watchdogTimerSec = 0.0f;
        }
        
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

    // --- ЛОГІКА ТЕРМАЛЬНОГО ЗАХИСТУ ---
    void HeaterChannel::checkThermalWatchdog(float dt) {
        // Якщо система "жарить" на високій потужності (> 80%)
        if (_pwmDuty >= WD_PWM_THRESHOLD) {
            
            // Захоплюємо температуру на початку відліку
            if (_watchdogTimerSec == 0.0f) {
                _watchdogCheckpointTemp = _currentTemp;
            }

            _watchdogTimerSec += dt;

            // Коли таймер вийшов
            if (_watchdogTimerSec >= WD_TIMEOUT_SEC) {
                int16_t tempRise = _currentTemp - _watchdogCheckpointTemp;

                // Якщо температура не зросла на заданий мінімум - це АВАРІЯ
                if (tempRise < WD_MIN_TEMP_RISE) {
                    Logger::fatal("SAFETY", "[%s] THERMAL RUNAWAY DETECTED! Shutting down.", _name);
                    setState(ChannelState::Error);
                } else {
                    // Температура росте нормально. Скидаємо таймер і чекпойнт.
                    _watchdogTimerSec = 0.0f;
                }
            }
        } else {
            // Якщо потужність падає нижче 80% - ПІД працює нормально, знімаємо "підозру"
            _watchdogTimerSec = 0.0f;
        }
    }

    void HeaterChannel::updateControlLoop(float dt) {
        if (_state == ChannelState::Off || _state == ChannelState::Error) {
            _pwmDuty = 0.0f;
            _pwmDriver.setDutyCycle(_pwmDuty);
            return;
        }

        if (_state == ChannelState::Active) {
            _idleTimeSec += dt;
            if (_sleepTimeoutSec > 0 && _idleTimeSec >= _sleepTimeoutSec) {
                Logger::info("LOGIC", "[%s] Auto-Sleep triggered due to inactivity", _name);
                setState(ChannelState::Sleep);
            }
        }

        int16_t activeTarget = (_state == ChannelState::Sleep) ? _sleepTemp : _targetTemp;
        
        _pwmDuty = _pid.compute((float)activeTarget, (float)_currentTemp, dt);

        checkThermalWatchdog(dt);

        if (_state == ChannelState::Error) {
            _pwmDuty = 0.0f;
        }

        _pwmDriver.setDutyCycle(_pwmDuty);
    }

} // namespace Hephaestus
