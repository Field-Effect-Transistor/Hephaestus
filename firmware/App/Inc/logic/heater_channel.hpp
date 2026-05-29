// App/Inc/logic/heater_channel.hpp
#pragma once

#include <cstdint>
#include "interfaces/IPwm.hpp"
#include "logic/pid_controller.hpp"

namespace Hephaestus {

    // Всі можливі стани нагрівального каналу
    enum class ChannelState {
        Off,      // Вимкнено (ШІМ = 0)
        Active,   // Робочий режим (ПІД працює, тримає цільову темп.)
        Sleep,    // Режим сну (знижена температура, наприклад 150°C)
        Error     // Аварія (відключено через перегрів або обрив датчика)
    };

    class HeaterChannel {
    private:
        const char*     _name;       // Назва для логування ("IRON" або "AIR")
        IPwm&           _pwmDriver;

        int16_t         _targetTemp; // Задана температура
        int16_t         _currentTemp;// Поточна (реальна) температура
        float           _pwmDuty;    // Відсоток потужності ШІМ (0-100)
        ChannelState    _state;      // Поточний стан
        PidController   _pid;

        const int16_t   _minTemp;   // Мінімальний ліміт температури
        const int16_t   _maxTemp;   // Максимальний ліміт температури
        int16_t         _sleepTemp;
        
        uint16_t        _sleepTimeoutSec; // Час до сну в секундах
        float           _idleTimeSec;     // Поточний час бездіяльності в секундах

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

        // Керування станом
        void toggleState();
        void setState(ChannelState newState);
        ChannelState getState() const { return _state; }

        // Керування температурою
        void changeTargetTemp(int16_t delta);
        void setTargetTemp(int16_t newTemp);
        
        void setCurrentTemp(int16_t temp) { _currentTemp = temp; }
        virtual void updateControlLoop(float dt);

        void forcePwmOff() { _pwmDriver.setDutyCycle(0.0f); }

        // Геттери
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
