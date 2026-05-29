#pragma once
#include <cstdint>
#include <atomic>
#include <cstring>

namespace Hephaestus {

    // Клас-адаптер для симуляції апаратури (OLED, Енкодери, Кнопки) на ПК.
    // Розділяє контексти: RTOS пише дані, Main Thread читає і малює.
    class SdlContext {
    private:
        // Використовуємо void*, щоб не підключати <SDL2/SDL.h> в інші частини проєкту
        void* _window   = nullptr;
        void* _renderer = nullptr;
        
        // Віртуальні стани апаратури
        std::atomic<bool>    _ironBtnPressed{false};
        std::atomic<bool>    _airBtnPressed{false};
        std::atomic<int16_t> _ironEncDiff{0};
        std::atomic<int16_t> _airEncDiff{0};
        std::atomic<bool>    _ironInHand{false}; 
        std::atomic<bool>    _airInHand{false}; 

        // Буфер екрану (128x64 / 8 біт = 1024 байти) та прапорець оновлення
        uint8_t           _framebuffer[128 * 8] = {};
        std::atomic<bool> _frameDirty{false};

        const int SCALE = 3; // Масштаб пікселів у вікні ПК

        void drawFrame(); // Викликається лише з Main Thread

    public:
        SdlContext() = default;
        ~SdlContext();

        void init();
        void runLoop(); // Блокуючий цикл (опитування подій + рендер)

        // Потокобезпечна передача кадру від u8g2 (викликається з RTOS)
        void submitBuffer(const uint8_t* buffer);

        // Інтерфейси для Mocks
        bool isIronPressed() const { return _ironBtnPressed; }
        bool isAirPressed()  const { return _airBtnPressed; }
        bool isIronInHand() const { return _ironInHand; }
        bool isAirInHand()  const { return _airInHand; }

        int16_t getIronEncDiff();
        int16_t getAirEncDiff();
    };

} // namespace Hephaestus
