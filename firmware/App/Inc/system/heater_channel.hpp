// App/Inc/system/heater_channel.hpp
#pragma once

#include <cstdint>

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
        const char*  _name;       // Назва для логування ("IRON" або "AIR")
        int16_t      _targetTemp; // Задана температура
        int16_t      _currentTemp;// Поточна (реальна) температура
        uint8_t      _pwmDuty;    // Відсоток потужності ШІМ (0-100)
        ChannelState _state;      // Поточний стан

        const int16_t _minTemp;   // Мінімальний ліміт температури
        const int16_t _maxTemp;   // Максимальний ліміт температури
        const int16_t _sleepTemp; // Температура для режиму сну

    public:
        HeaterChannel(const char* name, int16_t defaultTemp, int16_t minTemp, int16_t maxTemp, int16_t sleepTemp);

        // Керування станом
        void toggleState();
        void setState(ChannelState newState);
        ChannelState getState() const { return _state; }

        // Керування температурою
        void changeTargetTemp(int16_t delta);
        void setTargetTemp(int16_t newTemp);
        
        // Оновлення поточної температури (буде викликатись з АЦП задачі)
        void setCurrentTemp(int16_t temp) { _currentTemp = temp; }

        // Геттери
        int16_t getTargetTemp() const { return _targetTemp; }
        int16_t getCurrentTemp() const { return _currentTemp; }
        const char* getName() const { return _name; }
    };

} // namespace Hephaestus
