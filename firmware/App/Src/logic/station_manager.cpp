//  App/Src/logic/station_manager.cpp

#include "logic/station_manager.hpp"
#include "system/logger/logger.hpp"

namespace Hephaestus {

    StationManager::StationManager(
        IStorage& storage, IAdc& adc, 
        IDigitalPin& ironPin, IDigitalPin& airPin, 
        IDigitalPin& ironStandPin, IDigitalPin& airStandPin, 
        IEncoder& ironEnc, IEncoder& airEnc, 
        IPwm& ironPwm, IPwm& airPwm, IPwm& airFanPwm, IPwm& buzzerPwm)
        : _storage(storage), _adc(adc), 
          _ironPin(ironPin), _airPin(airPin), 
          _ironStandPin(ironStandPin), _airStandPin(airStandPin),
          _ironEncoder(ironEnc), _airEncoder(airEnc),
          _ironChannel("IRON", ironPwm, 300, 100, 450, 150, 
                       _sysConfig.sensors.ironKp, _sysConfig.sensors.ironKi, _sysConfig.sensors.ironKd, _sysConfig.sensors.ironSleepTimeoutSec),
          _airChannel("AIR", airPwm, airFanPwm, 300, 100, 500, 50, 
                      _sysConfig.sensors.airKp, _sysConfig.sensors.airKi, _sysConfig.sensors.airKd, _sysConfig.sensors.airSleepTimeoutSec),
          _buzzer(buzzerPwm), // Ініціалізуємо зумер
          _systemContext{_ironChannel, _airChannel, _sysConfig}
    {
        if (_storage.load(_sysConfig)) {
            Logger::info("SYS", "Configuration loaded from storage.");
        } else {
            Logger::warn("SYS", "Storage empty or corrupted. Using defaults.");
            _storage.save(_sysConfig); 
        }

        _ironChannel.setTargetTemp(_sysConfig.user.ironTargetTemp);
        _airChannel.setTargetTemp(_sysConfig.user.airTargetTemp);
        _airChannel.setFanSpeed(_sysConfig.user.airFanSpeed);
    }

    void StationManager::init() {
        _ironEncoder.init();
        _airEncoder.init();
        Logger::info("SYS", "StationManager initialized (Hardware)");
    }

    void StationManager::requestConfigSave() {
        _pendingSave = true;
        _saveCountdownSec = SAVE_DELAY_SEC;
    }

    void StationManager::processPendingSave(float dt) {
        if (!_pendingSave) return;
        
        _saveCountdownSec -= dt;
        
        if (_saveCountdownSec <= 0.0f) {
            _pendingSave = false;
            
            _sysConfig.user.ironTargetTemp = _ironChannel.getTargetTemp();
            _sysConfig.user.airTargetTemp  = _airChannel.getTargetTemp();
            _sysConfig.user.airFanSpeed    = _airChannel.getFanSpeed();
            
            if (_storage.save(_sysConfig)) {
                Logger::info("SYS", "Config saved to Flash (Deferred).");
            } else {
                Logger::error("SYS", "Config save to Flash FAILED!");
            }
        }
    }

    void StationManager::handleButton(HeaterChannel& channel, ButtonEvent event, bool isIron) {
        if (event == ButtonEvent::None) return;

        channel.resetIdleTimer();

        requestConfigSave();
        
        if (event == ButtonEvent::SingleClick || event == ButtonEvent::DoubleClick) {
            _buzzer.play(BuzzerSound::ShortBeep, HAL_GetTick());
        }

        channel.resetIdleTimer();

        if (_display.getCurrentScreen() != &screenMain) {
            if (isIron) {
                _display.dispatchButton(event);
            }
            return;
        }

        if (isIron) {
            IScreen* next = screenMain.handleButton(event, _systemContext);
            if (next) _display.setScreen(next);
        } else {
            IScreen* next = screenMain.handleAirButton(event, _systemContext);
            if (next) _display.setScreen(next);
        }
    }

    void StationManager::handleEncoder(HeaterChannel& channel, EncoderResult enc, bool isPressed, bool isIron) {
        if (!enc.hasMovement()) return;

        requestConfigSave();

        int16_t steps = isPressed ? (enc.raw > 0 ? 50 : -50) : enc.accelerated;

        if (_display.getCurrentScreen() != &screenMain) {
            if (isIron) _display.dispatchEncoder(enc.raw); 
            return;
        }

        if (isIron) {
            screenMain.handleEncoder(steps, _systemContext);
        } else {
            screenMain.handleAirEncoder(steps, _systemContext);
        }
    }

    void StationManager::initDisplay() {
        _display.init(&screenMain, &_systemContext);
        Logger::info("SYS", "Display initialized");
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

        if (_ironStandPin.isActive()) {
            _ironChannel.resetIdleTimer();
        }

        if (_airStandPin.isActive()) {
            _airChannel.setState(ChannelState::Sleep);
        } else {
            _airChannel.resetIdleTimer();
            if (_airChannel.getState() == ChannelState::Sleep) {
                _airChannel.setState(ChannelState::Active);
            }
        }
        
        _buzzer.tick(currentTickMs);
    }

    void StationManager::tickControl(uint32_t currentTickMs) {
        static uint32_t lastTickMs = 0; 
        float dt = (float)(currentTickMs - lastTickMs) / 1000.0f;
        lastTickMs = currentTickMs;
        if (dt <= 0.0f || dt > 1.0f) dt = 0.05f;

        processPendingSave(dt);

        _ironChannel.forcePwmOff();
        vTaskDelay(pdMS_TO_TICKS(2));

        float ironVolts = _adc.readVoltage(2);
        float airVolts  = _adc.readVoltage(3);
        float psuAdcVolts = _adc.readVoltage(0);
        float ntcAdcVolts = _adc.readVoltage(1);

        _systemContext.psuVoltage = MathSensors::calculatePsuVoltage(psuAdcVolts, _sysConfig.sensors);
        _systemContext.ambientTempC = MathSensors::calculateNtcTempC(ntcAdcVolts, _sysConfig.sensors);

        float ironTempC = MathSensors::calculateThermocoupleTemp(
            ironVolts, _sysConfig.sensors.ironOpAmpGain, _sysConfig.sensors.ironOpAmpOffsetV, 
            _systemContext.ambientTempC);
            
        ironTempC = MathSensors::applyCalibration(
            ironTempC, 
            _sysConfig.sensors.ironCalibRaw, 
            _sysConfig.sensors.ironCalibReal
        );

        _ironChannel.setCurrentTemp(static_cast<int16_t>(ironTempC));
            
        float airTempC  = MathSensors::calculateThermocoupleTemp(
            airVolts, _sysConfig.sensors.airOpAmpGain, _sysConfig.sensors.airOpAmpOffsetV, 
            _systemContext.ambientTempC);

        _ironChannel.setCurrentTemp(static_cast<int16_t>(ironTempC));
        _airChannel.setCurrentTemp(static_cast<int16_t>(airTempC));

        _ironChannel.updateControlLoop(dt);
        _airChannel.updateControlLoop(dt);
    }

    void StationManager::tickDisplay() {
        _display.update();
    }

} // namespace Hephaestus