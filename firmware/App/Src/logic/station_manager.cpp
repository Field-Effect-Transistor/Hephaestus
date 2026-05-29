//  App/Src/logic/station_manager.cpp

#include "logic/station_manager.hpp"
#include "system/logger/logger.hpp"

namespace Hephaestus {

    StationManager::StationManager(IAdc& adc, 
                                   IDigitalPin& ironPin, IDigitalPin& airPin, 
                                   IEncoder& ironEnc, IEncoder& airEnc, 
                                   IPwm& ironPwm, IPwm& airPwm)
        : _adc(adc), _ironPin(ironPin), _airPin(airPin), 
          _ironEncoder(ironEnc), _airEncoder(airEnc),
          _ironChannel("IRON", ironPwm, 300, 100, 450, 150, 
                       _sysConfig.sensors.ironKp, _sysConfig.sensors.ironKi, _sysConfig.sensors.ironKd),
          _airChannel("AIR", airPwm, 300, 100, 500, 50, 
                      _sysConfig.sensors.airKp, _sysConfig.sensors.airKi, _sysConfig.sensors.airKd),
                      _systemContext{_ironChannel, _airChannel, _sysConfig}
    {}

    void StationManager::init() {
        _ironEncoder.init();
        _airEncoder.init();
        Logger::info("SYS", "StationManager initialized (Hardware)");
    }

    void StationManager::handleButton(HeaterChannel& channel, ButtonEvent event, bool isIron) {
        // Завжди передаємо кнопку в дисплей (можливо, ми в меню і треба клікнути "Exit")
        if (isIron) {
            _display.dispatchButton(event);
        }

        // Якщо ми НА ГОЛОВНОМУ ЕКРАНІ, кнопки керують каналами
        if (_display.getCurrentScreen() == &screenMain) {
            if (event == ButtonEvent::SingleClick) {
                channel.toggleState(); 
            } else if (event == ButtonEvent::DoubleClick) {
                channel.setState(ChannelState::Sleep); 
            }
        }
    }

    void StationManager::initDisplay() {
        _display.init(&screenMain, &_systemContext);
        Logger::info("SYS", "Display initialized");
    }


    void StationManager::handleEncoder(HeaterChannel& channel, EncoderResult enc, bool isPressed, bool isIron) {
        if (!enc.hasMovement()) return;

        if (_display.getCurrentScreen() != &screenMain) {
            if (isIron) {
                _display.dispatchEncoder(enc.raw);
            }
            return;
        }

        // Якщо ми на головному екрані - керуємо температурою
        if (isPressed) {
            int16_t direction = (enc.raw > 0) ? 1 : -1;
            channel.changeTargetTemp(direction * 50);
        } else {
            channel.changeTargetTemp(enc.accelerated);
        }
    }
    
    void StationManager::tickInput(uint32_t currentTickMs) {
        bool isIronPressed = _ironPin.isActive();
        bool isAirPressed  = _airPin.isActive();

        ButtonEvent ironEvent = _ironBtn.update(isIronPressed, currentTickMs);
        ButtonEvent airEvent  = _airBtn.update(isAirPressed, currentTickMs);
        
        EncoderResult ironEnc = _ironEncoder.getSteps();
        EncoderResult airEnc  = _airEncoder.getSteps();

        handleButton(_ironChannel, ironEvent, true);
        handleButton(_airChannel, airEvent, false);
        handleEncoder(_ironChannel, ironEnc, isIronPressed, true);
        handleEncoder(_airChannel, airEnc, isAirPressed, false);
    }

    void StationManager::tickControl(uint32_t currentTickMs) {
        static uint32_t lastTickMs = 0; 
        float dt = (float)(currentTickMs - lastTickMs) / 1000.0f;
        lastTickMs = currentTickMs;
        if (dt <= 0.0f || dt > 1.0f) dt = 0.05f;

        // 1. АЛГОРИТМ ЧАСОВОГО РОЗДІЛЕННЯ (TDM)
        _ironChannel.forcePwmOff();
        vTaskDelay(pdMS_TO_TICKS(2));

        // 2. БЕЗПЕЧНЕ ЧИТАННЯ АЦП 
        float ironVolts = _adc.readVoltage(2);
        float airVolts  = _adc.readVoltage(3);
        float psuAdcVolts = _adc.readVoltage(0);
        float ntcAdcVolts = _adc.readVoltage(1);

        // 3. МАТЕМАТИКА
        _systemContext.psuVoltage = MathSensors::calculatePsuVoltage(psuAdcVolts, _sysConfig.sensors);
        _systemContext.ambientTempC = MathSensors::calculateNtcTempC(ntcAdcVolts, _sysConfig.sensors);

        float ironTempC = MathSensors::calculateThermocoupleTemp(
            ironVolts, _sysConfig.sensors.ironOpAmpGain, _sysConfig.sensors.ironOpAmpOffsetV, 
            _sysConfig.sensors.ironTcSensitivity, _systemContext.ambientTempC);
            
        float airTempC  = MathSensors::calculateThermocoupleTemp(
            airVolts, _sysConfig.sensors.airOpAmpGain, _sysConfig.sensors.airOpAmpOffsetV, 
            _sysConfig.sensors.airTcSensitivity, _systemContext.ambientTempC);

        _ironChannel.setCurrentTemp(static_cast<int16_t>(ironTempC));
        _airChannel.setCurrentTemp(static_cast<int16_t>(airTempC));

        // 4. ПІД-РЕГУЛЯТОР ТА УВІМКНЕННЯ ШІМ
        _ironChannel.updateControlLoop(dt);
        _airChannel.updateControlLoop(dt);
    }

    void StationManager::tickDisplay() {
        _display.update();
    }

} // namespace Hephaestus
