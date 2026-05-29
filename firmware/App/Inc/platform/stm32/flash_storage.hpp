// App/Inc/platform/stm32/flash_storage.hpp
#pragma once

#include "interfaces/IStorage.hpp"

namespace Hephaestus {

    class FlashStorage : public IStorage {
    private:
        // Використовуємо останню сторінку (Page 63) у 64KB Flash STM32F103C8T6
        static constexpr uint32_t FLASH_PAGE_ADDR = 0x0800FC00; 
        
        // Магічне число для перевірки, чи ми вже писали туди щось раніше
        static constexpr uint32_t MAGIC_WORD = 0xDEADBEEF;

        // Структура, яка реально пишеться у флеш (щоб перевіряти цілісність)
        struct FlashWrapper {
            uint32_t magic;
            SystemConfig config;
        };

    public:
        FlashStorage() = default;

        bool load(SystemConfig& config) override;
        bool save(const SystemConfig& config) override;
    };

} // namespace Hephaestus
