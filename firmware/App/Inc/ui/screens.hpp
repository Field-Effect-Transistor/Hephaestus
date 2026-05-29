// App/Inc/ui/screens.hpp

#pragma once
#include "interfaces/IScreen.hpp"
#include "logic/heater_channel.hpp"
#include <cstdio>

namespace Hephaestus {

    // --- ГОЛОВНИЙ ЕКРАН (Пайка) ---
    class ScreenMain : public IScreen {
    private:
        bool _isEditingAirFan = false; // false = Температура, true = Вентилятор

    public:
        void draw(u8g2_t* u8g2, const SystemContext& ctx) override;
        void handleEncoder(int16_t steps, SystemContext& ctx) override;
        IScreen* handleButton(ButtonEvent event, SystemContext& ctx) override;
        
        void handleAirEncoder(int16_t steps, SystemContext& ctx);
        IScreen* handleAirButton(ButtonEvent event, SystemContext& ctx);
    };

    // --- ЕКРАН ГОЛОВНОГО МЕНЮ ---
    class ScreenMenu : public IScreen {
    private:
        int8_t _cursorIdx = 0;
        int8_t _scrollOffset = 0; // Змінна для реалізації "камери" при прокрутці
        static const int8_t ITEMS_COUNT = 6;
        const char* _items[ITEMS_COUNT] = { 
            "Iron Setup", "Air Setup", "PID Tuning", 
            "Calibration", "System Info", "Exit" 
        };

    public:
        void draw(u8g2_t* u8g2, const SystemContext& ctx) override;
        void handleEncoder(int16_t steps, SystemContext& ctx) override;
        IScreen* handleButton(ButtonEvent event, SystemContext& ctx) override;
    };

    // --- ЕКРАН НАЛАШТУВАННЯ ПІД (PID Tuning) ---
    class ScreenPidTuning : public IScreen {
    private:
        int8_t _cursorIdx = 0; 
        bool _isEditing = false; // Прапорець: гортаємо меню чи змінюємо значення
        static const int8_t ITEMS_COUNT = 4; // Kp, Ki, Kd, Back

    public:
        void draw(u8g2_t* u8g2, const SystemContext& ctx) override;
        void handleEncoder(int16_t steps, SystemContext& ctx) override;
        IScreen* handleButton(ButtonEvent event, SystemContext& ctx) override;
    };

    // --- ЕКРАН СИСТЕМНОЇ ДІАГНОСТИКИ (System Info) ---
    class ScreenSystemInfo : public IScreen {
    public:
        void draw(u8g2_t* u8g2, const SystemContext& ctx) override;
        void handleEncoder(int16_t steps, SystemContext& ctx) override;
        IScreen* handleButton(ButtonEvent event, SystemContext& ctx) override;
    };

    // --- ЕКРАН НАЛАШТУВАННЯ ПАЯЛЬНИКА ---
    class ScreenIronSetup : public IScreen {
    private:
        int8_t _cursorIdx = 0; 
        bool _isEditing = false;
        static const int8_t ITEMS_COUNT = 3;

    public:
        void draw(u8g2_t* u8g2, const SystemContext& ctx) override;
        void handleEncoder(int16_t steps, SystemContext& ctx) override;
        IScreen* handleButton(ButtonEvent event, SystemContext& ctx) override;
    };

    // --- ЕКРАН НАЛАШТУВАННЯ ФЕНА ---
    class ScreenAirSetup : public IScreen {
    private:
        int8_t _cursorIdx = 0; 
        bool _isEditing = false;
        static const int8_t ITEMS_COUNT = 3; 

    public:
        void draw(u8g2_t* u8g2, const SystemContext& ctx) override;
        void handleEncoder(int16_t steps, SystemContext& ctx) override;
        IScreen* handleButton(ButtonEvent event, SystemContext& ctx) override;
    };

    // --- ЕКРАН КАЛІБРУВАННЯ (Calibration) ---
    class ScreenCalibration : public IScreen {
    public:
        void draw(u8g2_t* u8g2, const SystemContext& ctx) override;
        void handleEncoder(int16_t steps, SystemContext& ctx) override;
        IScreen* handleButton(ButtonEvent event, SystemContext& ctx) override;
    };

    extern ScreenIronSetup screenIronSetup;
    extern ScreenAirSetup screenAirSetup;
    extern ScreenMain screenMain;
    extern ScreenMenu screenMenu;
    extern ScreenPidTuning screenPidTuning;
    extern ScreenSystemInfo screenSystemInfo;
    extern ScreenCalibration screenCalibration;

} // namespace Hephaestus
