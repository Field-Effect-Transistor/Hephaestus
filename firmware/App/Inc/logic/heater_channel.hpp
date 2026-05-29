// App/Inc/logic/heater_channel.hpp
#pragma once

#include <cstdint>
#include "interfaces/IPwm.hpp"
#include "logic/pid_controller.hpp"

namespace Hephaestus {

    enum class ChannelState {
        Off,      
        Active,   
        Sleep,    
        Error     // Аварія (Відключено Watchdog-ом)
    };

    class HeaterChannel {
    private:
        const char*     _name;       
        IPwm&           _pwmDriver;

        int16_t         _targetTemp; 
        int16_t         _currentTemp;
        float           _pwmDuty;    
        ChannelState    _state;      
        PidController   _pid;

        const int16_t   _minTemp;   
        const int16_t   _maxTemp;   
        int16_t         _sleepTemp;
        
        uint16_t        _sleepTimeoutSec; 
        float           _idleTimeSec;     

        float           _watchdogTimerSec = 0.0f;
        int16_t         _watchdogCheckpointTemp = 0;
        
        // Налаштування Watchdog-а
        static constexpr float   WD_TIMEOUT_SEC = 3.0f;   // Час для перевірки (3 секунди)
        static constexpr int16_t WD_MIN_TEMP_RISE = 5;    // Мінімальний приріст температури за цей час
        static constexpr float   WD_PWM_THRESHOLD = 80.0f;// Мінімальний ШІМ, при якому вмикається перевірка

        void checkThermalWatchdog(float dt);

    public:
        virtual ~HeaterChannel() = default;
        HeaterChannel(
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
        );

        void toggleState();
        void setState(ChannelState newState);
        ChannelState getState() const { return _state; }

        void changeTargetTemp(int16_t delta);
        void setTargetTemp(int16_t newTemp);
        
        void setCurrentTemp(int16_t temp) { _currentTemp = temp; }
        virtual void updateControlLoop(float dt);

        void forcePwmOff() { _pwmDriver.setDutyCycle(0.0f); }

        int16_t getTargetTemp() const { return _targetTemp; }
        int16_t getCurrentTemp() const { return _currentTemp; }
        const char* getName() const { return _name; }
        void setSleepTemp(int16_t temp) { _sleepTemp = temp; }
        int16_t getSleepTemp() const { return _sleepTemp; }
        int16_t getMaxTemp() const { return _maxTemp; }

        void resetIdleTimer() { _idleTimeSec = 0.0f; }
        void setSleepTimeout(uint16_t sec) { _sleepTimeoutSec = sec; }
        uint16_t getSleepTimeout() const { return _sleepTimeoutSec; }
    };

} // namespace Hephaestus