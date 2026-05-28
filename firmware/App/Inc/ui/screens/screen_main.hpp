#pragma once
#include "interfaces/IScreen.hpp"
#include <cstdio>
#include "logic/heater_channel.hpp"

namespace Hephaestus {

    class ScreenMain : public IScreen {
    public:
        void draw(u8g2_t* u8g2, const SystemContext& ctx) override {
            char txt[16]; 
            uint8_t ro = 70;

            u8g2_DrawVLine(u8g2, 64, 0, 64);
            
            // ФЕН (Зліва)
            u8g2_SetFont(u8g2, u8g2_font_helvB08_tf);
            u8g2_DrawStr(u8g2, 0, 10, "AIR SET:");
            snprintf(txt, sizeof(txt), "%d", ctx.airChannel.getTargetTemp());
            u8g2_DrawStr(u8g2, 0, 22, txt);

            u8g2_SetFont(u8g2, u8g2_font_logisoso24_tn); 
            snprintf(txt, sizeof(txt), "%d", ctx.airChannel.getCurrentTemp());
            u8g2_DrawStr(u8g2, 0, 52, txt);

            // ПАЯЛЬНИК (Справа)
            u8g2_SetFont(u8g2, u8g2_font_helvB08_tf);
            u8g2_DrawStr(u8g2, ro, 10, "IRON SET:");
            snprintf(txt, sizeof(txt), "%d", ctx.ironChannel.getTargetTemp());
            u8g2_DrawStr(u8g2, ro, 22, txt);

            u8g2_SetFont(u8g2, u8g2_font_logisoso24_tn);
            snprintf(txt, sizeof(txt), "%d", ctx.ironChannel.getCurrentTemp());
            u8g2_DrawStr(u8g2, ro, 52, txt);
        }

        void handleEncoder(int16_t steps) override {
        }

        IScreen* handleButton(ButtonEvent event) override;
    };

}
