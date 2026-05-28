#pragma once
#include "interfaces/IScreen.hpp"

namespace Hephaestus {

    class ScreenMenu : public IScreen {
    private:
        int8_t _cursorIdx = 0;
        const int8_t _itemsCount = 4;
        const char* _items[4] = { "PID Tuning", "Calibration", "Sleep Temps", "Exit" };

    public:
        void draw(u8g2_t* u8g2, const SystemContext& ctx) override {
            u8g2_SetFont(u8g2, u8g2_font_helvB08_tf);
            u8g2_DrawStr(u8g2, 20, 10, "--- MAIN MENU ---");

            for (int i = 0; i < _itemsCount; i++) {
                uint8_t y = 25 + (i * 12);
                if (i == _cursorIdx) {
                    u8g2_DrawStr(u8g2, 5, y, ">");
                }
                u8g2_DrawStr(u8g2, 15, y, _items[i]);
            }
        }

        void handleEncoder(int16_t steps) override {
            _cursorIdx += steps;
            if (_cursorIdx < 0) _cursorIdx = 0;
            if (_cursorIdx >= _itemsCount) _cursorIdx = _itemsCount - 1;
        }

        IScreen* handleButton(ButtonEvent event) override;
    };

}
