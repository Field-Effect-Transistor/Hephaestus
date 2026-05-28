// App/Inc/ui/display_manager.hpp
#pragma once

#include "u8g2.h"
#include "interfaces/IScreen.hpp"

namespace Hephaestus {

    class DisplayManager {
    private:
        u8g2_t         _u8g2;
        IScreen*       _currentScreen;
        SystemContext* _context;

    public:
        DisplayManager() : _currentScreen(nullptr), _context(nullptr) {}

        void init(IScreen* initialScreen, SystemContext* ctx);
        void update();

        void dispatchEncoder(int16_t steps);
        void dispatchButton(ButtonEvent event);

        void setScreen(IScreen* newScreen) {
            if (newScreen) _currentScreen = newScreen;
        }

        IScreen* getCurrentScreen() const { return _currentScreen; }
    };

} // namespace Hephaestus
