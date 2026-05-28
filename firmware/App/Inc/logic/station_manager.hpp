//  App/Inc/logic/station_manager.hpp
#pragma once

#include "interfaces/IAdc.hpp"
#include "interfaces/IPwm.hpp"
#include "interfaces/IDigitalPin.hpp"
#include "interfaces/IEncoder.hpp"

#include "logic/button.hpp"
#include "logic/heater_channel.hpp"
#include "ui/display_manager.hpp"
#include "system/system_config.hpp"
#include "system/math_sensors.hpp"

namespace Hephaestus {

    class StationManager {
    private:
        SystemConfig   _sysConfig; 

        IAdc&        _adc;
        IDigitalPin& _ironPin;
        IDigitalPin& _airPin;
        IEncoder&    _ironEncoder;
        IEncoder&    _airEncoder;

        Button        _ironBtn;
        Button        _airBtn;
        
        HeaterChannel _ironChannel;
        HeaterChannel _airChannel;
        
        DisplayManager _display;

        void handleButton(HeaterChannel& channel, ButtonEvent event);
        void handleEncoder(HeaterChannel& channel, EncoderResult enc, bool isPressed);

    public:
        StationManager(IAdc& adc, 
                       IDigitalPin& ironPin, IDigitalPin& airPin, 
                       IEncoder& ironEnc, IEncoder& airEnc, 
                       IPwm& ironPwm, IPwm& airPwm);

        void init();
        void initDisplay();
        
        void tickInput(uint32_t currentTickMs); 
        void tickDisplay();                     
    };

} // namespace Hephaestus