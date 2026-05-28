// App/Src/ui/screens.cpp
#include "ui/screens.hpp"
#include "system/system_config.hpp"

namespace Hephaestus {

    ScreenMain screenMain;
    ScreenMenu screenMenu;
    ScreenPidTuning screenPidTuning;
    ScreenSystemInfo screenSystemInfo;

    // ==========================================
    // SCREEN MAIN
    // ==========================================
    void ScreenMain::draw(u8g2_t* u8g2, const SystemContext& ctx) {
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

        u8g2_SetFont(u8g2, u8g2_font_helvB08_tf);
        const char* airState = (ctx.airChannel.getState() == ChannelState::Active) ? "ACTIVE" : 
                               (ctx.airChannel.getState() == ChannelState::Sleep) ? "SLEEP" : "OFF";
        u8g2_DrawStr(u8g2, 0, 64, airState);
        
        // ПАЯЛЬНИК (Справа)
        u8g2_SetFont(u8g2, u8g2_font_helvB08_tf);
        u8g2_DrawStr(u8g2, ro, 10, "IRON SET:");
        snprintf(txt, sizeof(txt), "%d", ctx.ironChannel.getTargetTemp());
        u8g2_DrawStr(u8g2, ro, 22, txt);

        u8g2_SetFont(u8g2, u8g2_font_logisoso24_tn);
        snprintf(txt, sizeof(txt), "%d", ctx.ironChannel.getCurrentTemp());
        u8g2_DrawStr(u8g2, ro, 52, txt);

        u8g2_SetFont(u8g2, u8g2_font_helvB08_tf);
        const char* ironState = (ctx.ironChannel.getState() == ChannelState::Active) ? "ACTIVE" : 
                                (ctx.ironChannel.getState() == ChannelState::Sleep) ? "SLEEP" : "OFF";
        u8g2_DrawStr(u8g2, ro, 64, ironState);
    }

    void ScreenMain::handleEncoder(int16_t steps, SystemContext& ctx) {}

    IScreen* ScreenMain::handleButton(ButtonEvent event, SystemContext& ctx) {
        if (event == ButtonEvent::LongPress) return &screenMenu;
        return nullptr; 
    }

    // ==========================================
    // SCREEN MENU
    // ==========================================
    void ScreenMenu::draw(u8g2_t* u8g2, const SystemContext& ctx) {
        u8g2_SetFont(u8g2, u8g2_font_helvB08_tf);
        u8g2_DrawStr(u8g2, 20, 10, "--- MAIN MENU ---");

        for (int i = 0; i < 4; i++) {
            int itemIdx = i + _scrollOffset;
            if (itemIdx >= ITEMS_COUNT) break;

            uint8_t y = 25 + (i * 12);
            if (itemIdx == _cursorIdx) {
                u8g2_DrawStr(u8g2, 5, y, ">");
            }
            u8g2_DrawStr(u8g2, 15, y, _items[itemIdx]);
        }
    }

    void ScreenMenu::handleEncoder(int16_t steps, SystemContext& ctx) {
        _cursorIdx += steps;
        if (_cursorIdx < 0) _cursorIdx = 0;
        if (_cursorIdx >= ITEMS_COUNT) _cursorIdx = ITEMS_COUNT - 1;

        if (_cursorIdx < _scrollOffset) {
            _scrollOffset = _cursorIdx;
        } else if (_cursorIdx >= _scrollOffset + 4) {
            _scrollOffset = _cursorIdx - 3;
        }
    }

    IScreen* ScreenMenu::handleButton(ButtonEvent event, SystemContext& ctx) {
        if (event == ButtonEvent::SingleClick) {
            switch (_cursorIdx) {
                case 2: return &screenPidTuning;
                case 4: return &screenSystemInfo;
                case 5: 
                    _cursorIdx = 0; 
                    _scrollOffset = 0;
                    return &screenMain;
                default: break; 
            }
        }
        return nullptr;
    }

    // ==========================================
    // SCREEN PID TUNING
    // ==========================================
    void ScreenPidTuning::draw(u8g2_t* u8g2, const SystemContext& ctx) {
        u8g2_SetFont(u8g2, u8g2_font_helvB08_tf);
        u8g2_DrawStr(u8g2, 15, 10, "[ IRON PID TUNING ]");

        char buf[32];
        const float vals[3] = { ctx.config.sensors.ironKp, ctx.config.sensors.ironKi, ctx.config.sensors.ironKd };
        const char* names[3] = { "Kp", "Ki", "Kd" };

        for (int i = 0; i < 3; i++) {
            uint8_t y = 25 + (i * 12);
            char cursorChar = (_cursorIdx == i) ? (_isEditing ? '*' : '>') : ' ';
            snprintf(buf, sizeof(buf), "%c %s: %.2f", cursorChar, names[i], vals[i]);
            u8g2_DrawStr(u8g2, 5, y, buf);
        }

        uint8_t backY = 25 + (3 * 12);
        snprintf(buf, sizeof(buf), "%c Back", (_cursorIdx == 3) ? '>' : ' ');
        u8g2_DrawStr(u8g2, 5, backY, buf);
    }

    void ScreenPidTuning::handleEncoder(int16_t steps, SystemContext& ctx) {
        if (!_isEditing) {
            _cursorIdx += steps;
            if (_cursorIdx < 0) _cursorIdx = 0;
            if (_cursorIdx >= ITEMS_COUNT) _cursorIdx = ITEMS_COUNT - 1;
        } else {
            float delta = steps * 0.1f; 
            if (_cursorIdx == 0) ctx.config.sensors.ironKp += delta;
            else if (_cursorIdx == 1) ctx.config.sensors.ironKi += delta;
            else if (_cursorIdx == 2) ctx.config.sensors.ironKd += (steps * 0.01f); 

            if (ctx.config.sensors.ironKp < 0.0f) ctx.config.sensors.ironKp = 0.0f;
            if (ctx.config.sensors.ironKi < 0.0f) ctx.config.sensors.ironKi = 0.0f;
            if (ctx.config.sensors.ironKd < 0.0f) ctx.config.sensors.ironKd = 0.0f;
        }
    }

    IScreen* ScreenPidTuning::handleButton(ButtonEvent event, SystemContext& ctx) {
        if (event == ButtonEvent::SingleClick) {
            if (_cursorIdx == 3) {
                _cursorIdx = 0; 
                _isEditing = false;
                return &screenMenu;
            } else {
                _isEditing = !_isEditing; 
            }
        }
        return nullptr;
    }

    // ==========================================
    // SCREEN SYSTEM INFO
    // ==========================================
    void ScreenSystemInfo::draw(u8g2_t* u8g2, const SystemContext& ctx) {
        u8g2_SetFont(u8g2, u8g2_font_helvB08_tf);
        u8g2_DrawStr(u8g2, 20, 10, "--- SYSTEM INFO ---");

        char buf[32];
        snprintf(buf, sizeof(buf), "PSU Volt: %.2f V", ctx.psuVoltage);
        u8g2_DrawStr(u8g2, 5, 30, buf);

        snprintf(buf, sizeof(buf), "Ambient: %.1f C", ctx.ambientTempC);
        u8g2_DrawStr(u8g2, 5, 45, buf);

        u8g2_DrawStr(u8g2, 5, 60, "> Back (Click)");
    }

    void ScreenSystemInfo::handleEncoder(int16_t steps, SystemContext& ctx) {
        // На цьому екрані немає чого гортати
    }

    IScreen* ScreenSystemInfo::handleButton(ButtonEvent event, SystemContext& ctx) {
        if (event == ButtonEvent::SingleClick) {
            return &screenMenu;
        }
        return nullptr;
    }

} // namespace Hephaestus
