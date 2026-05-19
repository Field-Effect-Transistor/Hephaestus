#pragma once

#include "u8g2.h"
#include "system/heater_channel.hpp"

namespace Hephaestus {

    class DisplayManager {
    private:
        u8g2_t _u8g2;

    public:
        DisplayManager() = default;

        void init();

        void update(const HeaterChannel& iron, const HeaterChannel& air);
    };

} // namespace Hephaestus
