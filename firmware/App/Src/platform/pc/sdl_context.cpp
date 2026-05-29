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
        
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);
        SDL_SetRenderDrawColor(ren, 0, 200, 255, 255);

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
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) exit(0);

                if (e.type == SDL_KEYDOWN) {
                    switch (e.key.keysym.sym) {
                        case SDLK_RETURN: _ironBtnPressed.store(true);  break;
                        case SDLK_SPACE:  _airBtnPressed.store(true);   break;
                        case SDLK_UP:     _ironEncDiff.fetch_add(1);    break;
                        case SDLK_DOWN:   _ironEncDiff.fetch_sub(1);    break;
                        case SDLK_w:      _airEncDiff.fetch_add(1);     break;
                        case SDLK_s:      _airEncDiff.fetch_sub(1);     break;
                        case SDLK_i:      _ironInHand.store(!_ironInHand.load()); break;
                        case SDLK_a:      _airInHand.store(!_airInHand.load());  break;
                    }
                }
                if (e.type == SDL_KEYUP) {
                    switch (e.key.keysym.sym) {
                        case SDLK_RETURN: _ironBtnPressed.store(false); break;
                        case SDLK_SPACE:  _airBtnPressed.store(false);  break;
                    }
                }
            }

            if (_frameDirty.exchange(false)) {
                drawFrame();
            }

            SDL_Delay(10);
        }
    }

    void SdlContext::submitBuffer(const uint8_t* buffer) {
        if (!buffer) return;
        std::memcpy(_framebuffer, buffer, sizeof(_framebuffer));
        _frameDirty.store(true);
    }

    int16_t SdlContext::getIronEncDiff() {
        return _ironEncDiff.exchange(0);
    }
    
    int16_t SdlContext::getAirEncDiff() {
        return _airEncDiff.exchange(0);
    }

} // namespace Hephaestus
