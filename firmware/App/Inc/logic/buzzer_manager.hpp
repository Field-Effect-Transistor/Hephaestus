#pragma once

#include "interfaces/IPwm.hpp"
#include <cstdint>

namespace Hephaestus {

    enum class BuzzerSound {
        None,
        ShortBeep,  // Клік кнопки (50 мс)
        LongBeep,   // Збереження / Увімкнення (200 мс)
        Alarm       // Watchdog / Помилка (безперервно)
    };

    class BuzzerManager {
    private:
        IPwm&    _pwm;
        uint32_t _stopTimeMs; // Час (у тіках), коли треба вимкнути звук
        bool     _isPlaying;
        bool     _isAlarm;

    public:
        explicit BuzzerManager(IPwm& pwm) 
            : _pwm(pwm), _stopTimeMs(0), _isPlaying(false), _isAlarm(false) {
            _pwm.enable(false);
            _pwm.setDutyCycle(0.0f);
        }

        void play(BuzzerSound sound, uint32_t currentTickMs) {
            if (_isAlarm) return; // Аварію не можна перебити звичайним звуком

            switch (sound) {
                case BuzzerSound::ShortBeep:
                    _stopTimeMs = currentTickMs + 50;
                    _pwm.setDutyCycle(50.0f); // 50% меандр для максимальної гучності
                    _pwm.enable(true);
                    _isPlaying = true;
                    break;

                case BuzzerSound::LongBeep:
                    _stopTimeMs = currentTickMs + 200;
                    _pwm.setDutyCycle(50.0f);
                    _pwm.enable(true);
                    _isPlaying = true;
                    break;

                case BuzzerSound::Alarm:
                    _isAlarm = true;
                    _pwm.setDutyCycle(50.0f);
                    _pwm.enable(true);
                    break;

                default:
                    break;
            }
        }

        void stopAlarm() {
            _isAlarm = false;
            _pwm.enable(false);
            _isPlaying = false;
        }

        // Викликається кожні 15мс з Input Loop
        void tick(uint32_t currentTickMs) {
            if (_isPlaying && !_isAlarm && currentTickMs >= _stopTimeMs) {
                _pwm.enable(false);
                _isPlaying = false;
            }
        }
    };

} // namespace Hephaestus
