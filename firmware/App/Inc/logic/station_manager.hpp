//  App/Inc/logic/station_manager.hpp
#pragma once

#include "interfaces/IAdc.hpp"
#include "interfaces/IPwm.hpp"
#include "interfaces/IDigitalPin.hpp"
#include "interfaces/IEncoder.hpp"
#include "interfaces/IStorage.hpp"

#include "logic/button.hpp"
#include "logic/heater_channel.hpp"
#include "logic/hot_air_channel.hpp"
#include "ui/display_manager.hpp"
#include "ui/screens.hpp"
#include "system/system_config.hpp"
#include "system/math_sensors.hpp"

namespace Hephaestus {

    class StationManager {
    private:
        IStorage&      _storage;
        SystemConfig   _sysConfig; 
        SystemContext  _systemContext;

        IAdc&        _adc;
        IDigitalPin& _ironPin;
        IDigitalPin& _airPin;
        IDigitalPin& _ironStandPin; 
        IDigitalPin& _airStandPin;
        IEncoder&    _ironEncoder;
        IEncoder&    _airEncoder;

        Button        _ironBtn;
        Button        _airBtn;
        
        HeaterChannel _ironChannel;
        HotAirChannel _airChannel;
        
        DisplayManager _display;

    void handleButton(HeaterChannel& channel, ButtonEvent event, bool isIron);
    void handleEncoder(HeaterChannel& channel, EncoderResult enc, bool isPressed, bool isIron);

    public:
        StationManager( IStorage& storage,
                        IAdc& adc, 
                        IDigitalPin& ironPin, IDigitalPin& airPin, 
                        IDigitalPin& ironStandPin, IDigitalPin& airStandPin,
                        IEncoder& ironEnc, IEncoder& airEnc, 
                        IPwm& ironPwm, IPwm& airPwm, IPwm& airFanPwm);

        void init();
        void initDisplay();
        
        void tickInput(uint32_t currentTickMs); 
        void tickDisplay();                     
        void tickControl(uint32_t currentTickMs);
    };

} // namespace Hephaestus