#pragma once

#include <cstdint>
#include "u8g2.h"
#include "logic/button.hpp"

namespace Hephaestus {

    class HeaterChannel;
    struct SystemConfig;

    struct SystemContext {
        HeaterChannel& ironChannel;
        HeaterChannel& airChannel;
        SystemConfig&  config;
        
        float psuVoltage = 0.0f;
        float ambientTempC = 0.0f;
    };

    class IScreen {
    public:
        virtual ~IScreen() = default;

        virtual void draw(u8g2_t* u8g2, const SystemContext& context) = 0;
        
        virtual void handleEncoder(int16_t steps, SystemContext& context) = 0;
        virtual IScreen* handleButton(ButtonEvent event, SystemContext& context) = 0;
    };

} // namespace Hephaestus