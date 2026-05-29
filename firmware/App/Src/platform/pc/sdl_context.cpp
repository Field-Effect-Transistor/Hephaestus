#include "platform/pc/sdl_context.hpp"
#include <SDL2/SDL.h>
#include <cstdlib>
#include <iostream>

namespace Hephaestus {

    SdlContext::~SdlContext() {
        if (_renderer) SDL_DestroyRenderer(static_cast<SDL_Renderer*>(_renderer));
        if (_window)   SDL_DestroyWindow(static_cast<SDL_Window*>(_window));
        SDL_Quit();
    }

    void SdlContext::init() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL Init Failed: " << SDL_GetError() << "\n";
            exit(1);
        }
        _window = SDL_CreateWindow(
            "Hephaestus Simulator",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            128 * SCALE, 64 * SCALE, 0);
        _renderer = SDL_CreateRenderer(
            static_cast<SDL_Window*>(_window), -1, SDL_RENDERER_ACCELERATED);
    }

    void SdlContext::drawFrame() {
        auto ren = static_cast<SDL_Renderer*>(_renderer);
        
        // Очищення фону
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);
        
        // Колір пікселів дисплея
        SDL_SetRenderDrawColor(ren, 0, 200, 255, 255);

        // Парсинг пам'яті u8g2: 8 сторінок по вертикалі, 128 колонок.
        // Кожен байт кодує стовпчик з 8 пікселів (LSB зверху).
        for (int y = 0; y < 64; y++) {
            for (int x = 0; x < 128; x++) {
                int page = y / 8;
                int bit = y % 8;
                if (_framebuffer[page * 128 + x] & (1 << bit)) {
                    SDL_Rect r = {x * SCALE, y * SCALE, SCALE, SCALE};
                    SDL_RenderFillRect(ren, &r);
                }
            }
        }
        SDL_RenderPresent(ren);
    }

    void SdlContext::runLoop() {
        SDL_Event e;
        while (true) {
            // 1. Обробка вводу (SDL вимагає робити це в тому ж потоці, де створено вікно)
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) exit(0);

                if (e.type == SDL_KEYDOWN) {
                    switch (e.key.keysym.sym) {
                        case SDLK_RETURN: _ironBtnPressed = true;  break;
                        case SDLK_SPACE:  _airBtnPressed  = true;  break;
                        case SDLK_UP:     _ironEncDiff = 1;  break;
                        case SDLK_DOWN:   _ironEncDiff = -1; break;
                        case SDLK_w:      _airEncDiff  = 1;  break;
                        case SDLK_s:      _airEncDiff  = -1; break;
                        case SDLK_i:      _ironInHand = !_ironInHand; break;
                        case SDLK_a:      _airInHand  = !_airInHand;  break;
                    }
                }
                if (e.type == SDL_KEYUP) {
                    switch (e.key.keysym.sym) {
                        case SDLK_RETURN: _ironBtnPressed = false; break;
                        case SDLK_SPACE:  _airBtnPressed  = false; break;
                    }
                }
            }

            // 2. Відмальовка нового кадру (якщо RTOS-задача надіслала дані)
            if (_frameDirty.exchange(false)) {
                drawFrame();
            }

            // Розвантажуємо ядро ПК
            SDL_Delay(10);
        }
    }

    void SdlContext::submitBuffer(const uint8_t* buffer) {
        if (!buffer) return;
        // Швидке копіювання буфера з потоку RTOS
        std::memcpy(_framebuffer, buffer, sizeof(_framebuffer));
        _frameDirty.store(true); // Сигналізуємо Main Thread, що можна малювати
    }

    // Забираємо дельту енкодерів і відразу очищаємо
    int16_t SdlContext::getIronEncDiff() {
        int16_t d = _ironEncDiff; 
        _ironEncDiff = 0; 
        return d;
    }
    
    int16_t SdlContext::getAirEncDiff() {
        int16_t d = _airEncDiff; 
        _airEncDiff = 0; 
        return d;
    }

} // namespace Hephaestus
