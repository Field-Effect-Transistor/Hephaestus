// App/Src/ui/screens.cpp
#include "ui/screens.hpp"
#include "system/system_config.hpp"
#include "logic/hot_air_channel.hpp"

namespace Hephaestus {

    ScreenMain screenMain;
    ScreenMenu screenMenu;
    ScreenPidTuning screenPidTuning;
    ScreenSystemInfo screenSystemInfo;
    ScreenIronSetup screenIronSetup;
    ScreenAirSetup screenAirSetup;

    // ==========================================
    // SCREEN MAIN
    // ==========================================
    #include "logic/hot_air_channel.hpp" // ОБОВ'ЯЗКОВО ДОДАЙ ЦЕЙ ІНКЛЮД ЗВЕРХУ ФАЙЛУ!

    void ScreenMain::draw(u8g2_t* u8g2, const SystemContext& ctx) {
        char txt[16]; 
        uint8_t ro = 70; // Відступ для паяльника

        u8g2_DrawVLine(u8g2, 64, 0, 64);
        
        // --- ФЕН (Зліва) ---
        u8g2_SetFont(u8g2, u8g2_font_helvB08_tf);
        
        // Малюємо курсор (стрілочку), щоб показати, що ми редагуємо
        if (!_isEditingAirFan) u8g2_DrawStr(u8g2, 0, 10, ">");
        u8g2_DrawStr(u8g2, 8, 10, "AIR SET:");
        
        snprintf(txt, sizeof(txt), "%d", ctx.airChannel.getTargetTemp());
        u8g2_DrawStr(u8g2, 8, 22, txt);

        // Малюємо швидкість вентилятора
        if (_isEditingAirFan) u8g2_DrawStr(u8g2, 0, 34, ">");
        snprintf(txt, sizeof(txt), "FAN: %d%%", (int)ctx.airChannel.getFanSpeed());
        u8g2_DrawStr(u8g2, 8, 34, txt);

        // Поточна температура
        u8g2_SetFont(u8g2, u8g2_font_logisoso24_tn); 
        snprintf(txt, sizeof(txt), "%d", ctx.airChannel.getCurrentTemp());
        u8g2_DrawStr(u8g2, 0, 64, txt); // Зсунули вниз через вентилятор
        
        // --- ПАЯЛЬНИК (Справа) ---
        u8g2_SetFont(u8g2, u8g2_font_helvB08_tf);
        u8g2_DrawStr(u8g2, ro, 10, "IRON SET:");
        snprintf(txt, sizeof(txt), "%d", ctx.ironChannel.getTargetTemp());
        u8g2_DrawStr(u8g2, ro, 22, txt);

        u8g2_SetFont(u8g2, u8g2_font_logisoso24_tn);
        snprintf(txt, sizeof(txt), "%d", ctx.ironChannel.getCurrentTemp());
        u8g2_DrawStr(u8g2, ro, 64, txt);
    }

    // Обробка енкодера ПАЯЛЬНИКА
    void ScreenMain::handleEncoder(int16_t steps, SystemContext& ctx) {
        ctx.ironChannel.changeTargetTemp(steps);
    }

    // Обробка кнопки ПАЯЛЬНИКА
    IScreen* ScreenMain::handleButton(ButtonEvent event, SystemContext& ctx) {
        if (event == ButtonEvent::SingleClick) {
            ctx.ironChannel.toggleState();
        } else if (event == ButtonEvent::LongPress) {
            return &screenMenu; // Довге натискання йде в меню
        }
        return nullptr; 
    }

    // Обробка енкодера ФЕНА
    void ScreenMain::handleAirEncoder(int16_t steps, SystemContext& ctx) {
        if (_isEditingAirFan) {
            // Змінюємо швидкість вентилятора (по 5% за крок)
            float newSpeed = ctx.airChannel.getFanSpeed() + (steps * 5.0f);
            ctx.airChannel.setFanSpeed(newSpeed);
        } else {
            // Змінюємо температуру
            ctx.airChannel.changeTargetTemp(steps);
        }
    }

    // Обробка кнопки ФЕНА
    IScreen* ScreenMain::handleAirButton(ButtonEvent event, SystemContext& ctx) {
        if (event == ButtonEvent::SingleClick) {
            ctx.airChannel.toggleState();
        } else if (event == ButtonEvent::DoubleClick) {
            // Подвійний клік перемикає фокус (Температура <-> Вентилятор)
            _isEditingAirFan = !_isEditingAirFan;
        } else if (event == ButtonEvent::LongPress) {
            ctx.airChannel.setState(ChannelState::Sleep);
        }
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
                case 0: return &screenIronSetup; // Iron Setup
                case 1: return &screenAirSetup;  // Air Setup
                case 2: return &screenPidTuning; // PID Tuning
                case 4: return &screenSystemInfo;// System Info
                case 5: // Exit
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

    void ScreenSystemInfo::handleEncoder(int16_t steps, SystemContext& ctx) {}

    IScreen* ScreenSystemInfo::handleButton(ButtonEvent event, SystemContext& ctx) {
        if (event == ButtonEvent::SingleClick) return &screenMenu;
        return nullptr;
    }

    // ==========================================
    // SCREEN IRON SETUP
    // ==========================================
    void ScreenIronSetup::draw(u8g2_t* u8g2, const SystemContext& ctx) {
        u8g2_SetFont(u8g2, u8g2_font_helvB08_tf);
        u8g2_DrawStr(u8g2, 20, 10, "[ IRON SETUP ]");

        char buf[32];
        
        char cursorChar0 = (_cursorIdx == 0) ? (_isEditing ? '*' : '>') : ' ';
        snprintf(buf, sizeof(buf), "%c Sleep Tmp: %d", cursorChar0, ctx.ironChannel.getSleepTemp());
        u8g2_DrawStr(u8g2, 5, 25, buf);

        char cursorChar1 = (_cursorIdx == 1) ? (_isEditing ? '*' : '>') : ' ';
        snprintf(buf, sizeof(buf), "%c Max Tmp: %d", cursorChar1, ctx.ironChannel.getMaxTemp());
        u8g2_DrawStr(u8g2, 5, 37, buf);

        snprintf(buf, sizeof(buf), "%c Back", (_cursorIdx == 2) ? '>' : ' ');
        u8g2_DrawStr(u8g2, 5, 49, buf);
    }

    void ScreenIronSetup::handleEncoder(int16_t steps, SystemContext& ctx) {
        if (!_isEditing) {
            _cursorIdx += steps;
            if (_cursorIdx < 0) _cursorIdx = 0;
            if (_cursorIdx >= ITEMS_COUNT) _cursorIdx = ITEMS_COUNT - 1;
        } else {
            if (_cursorIdx == 0) {
                int16_t newSleep = ctx.ironChannel.getSleepTemp() + steps * 5;
                if (newSleep < 50) newSleep = 50;
                if (newSleep > 300) newSleep = 300;
                ctx.ironChannel.setSleepTemp(newSleep);
            }
        }
    }

    IScreen* ScreenIronSetup::handleButton(ButtonEvent event, SystemContext& ctx) {
        if (event == ButtonEvent::SingleClick) {
            if (_cursorIdx == 2) {
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
    // SCREEN AIR SETUP
    // ==========================================
    void ScreenAirSetup::draw(u8g2_t* u8g2, const SystemContext& ctx) {
        u8g2_SetFont(u8g2, u8g2_font_helvB08_tf);
        u8g2_DrawStr(u8g2, 20, 10, "[ AIR SETUP ]");

        char buf[32];
        
        char cursorChar0 = (_cursorIdx == 0) ? (_isEditing ? '*' : '>') : ' ';
        snprintf(buf, sizeof(buf), "%c Sleep Tmp: %d", cursorChar0, ctx.airChannel.getSleepTemp());
        u8g2_DrawStr(u8g2, 5, 25, buf);

        snprintf(buf, sizeof(buf), "%c Back", (_cursorIdx == 1) ? '>' : ' ');
        u8g2_DrawStr(u8g2, 5, 37, buf);
    }

    void ScreenAirSetup::handleEncoder(int16_t steps, SystemContext& ctx) {
        if (!_isEditing) {
            _cursorIdx += steps;
            if (_cursorIdx < 0) _cursorIdx = 0;
            if (_cursorIdx >= ITEMS_COUNT) _cursorIdx = ITEMS_COUNT - 1;
        } else {
            if (_cursorIdx == 0) {
                int16_t newSleep = ctx.airChannel.getSleepTemp() + steps * 5;
                if (newSleep < 50) newSleep = 50;
                if (newSleep > 300) newSleep = 300;
                ctx.airChannel.setSleepTemp(newSleep);
            }
        }
    }

    IScreen* ScreenAirSetup::handleButton(ButtonEvent event, SystemContext& ctx) {
        if (event == ButtonEvent::SingleClick) {
            if (_cursorIdx == 1) {
                _cursorIdx = 0; 
                _isEditing = false;
                return &screenMenu;
            } else {
                _isEditing = !_isEditing; 
            }
        }
        return nullptr;
    }

} // namespace Hephaestus
