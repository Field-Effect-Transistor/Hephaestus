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
          _ironChannel("IRON", ironPwm, 300, 100, 450, 150),
          _airChannel("AIR", airPwm, 300, 100, 500, 50)
    {}

void StationManager::init() {
        _ironEncoder.init();
        _airEncoder.init();
        Logger::info("SYS", "StationManager initialized (Hardware)");
    }

    void StationManager::handleButton(HeaterChannel& channel, ButtonEvent event) {
        if (event == ButtonEvent::SingleClick) {
            channel.toggleState(); 
        } 
        else if (event == ButtonEvent::DoubleClick || event == ButtonEvent::LongPress) {
            channel.setState(ChannelState::Sleep); 
        }
    }

    void StationManager::initDisplay() {
        _display.init();
        Logger::info("SYS", "Display initialized");
    }


    void StationManager::handleEncoder(HeaterChannel& channel, EncoderResult enc, bool isPressed) {
        if (!enc.hasMovement()) return;
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

        handleButton(_ironChannel, ironEvent);
        handleButton(_airChannel, airEvent);
        handleEncoder(_ironChannel, ironEnc, isIronPressed);
        handleEncoder(_airChannel, airEnc, isAirPressed);

        float ironVolts = _adc.readVoltage(2);
        float airVolts  = _adc.readVoltage(3);

        float ironTempC = MathSensors::calculateThermocoupleTemp(ironVolts, _sysConfig.sensors.ironOpAmpGain, 0, _sysConfig.sensors.ironTcSensitivity, 25.0f);
        float airTempC  = MathSensors::calculateThermocoupleTemp(airVolts, _sysConfig.sensors.airOpAmpGain, 0, _sysConfig.sensors.airTcSensitivity, 25.0f);

        _ironChannel.setCurrentTemp(static_cast<int16_t>(ironTempC));
        _airChannel.setCurrentTemp(static_cast<int16_t>(airTempC));

        _ironChannel.updateControlLoop();
        _airChannel.updateControlLoop();
    }

    void StationManager::tickDisplay() {
        _display.update(_ironChannel, _airChannel);
    }

} // namespace Hephaestus
