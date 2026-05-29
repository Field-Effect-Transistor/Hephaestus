// App/Src/platform/stm32/flash_storage.cpp
#include "platform/stm32/flash_storage.hpp"
#include "stm32f1xx_hal.h"
#include <cstring>

namespace Hephaestus {

    bool FlashStorage::load(SystemConfig& config) {
        const FlashWrapper* flashData = reinterpret_cast<const FlashWrapper*>(FLASH_PAGE_ADDR);
        
        if (flashData->magic != MAGIC_WORD) {
            return false;
        }

        std::memcpy(&config, &flashData->config, sizeof(SystemConfig));
        return true;
    }

    bool FlashStorage::save(const SystemConfig& config) {
        FlashWrapper wrapper;
        wrapper.magic = MAGIC_WORD;
        std::memcpy(&wrapper.config, &config, sizeof(SystemConfig));

        uint32_t wordsToWrite = (sizeof(FlashWrapper) + 3) / 4; 
        uint32_t* dataPtr = reinterpret_cast<uint32_t*>(&wrapper);

        // 1. РОЗБЛОКУВАННЯ FLASH
        HAL_FLASH_Unlock();

        // 2. СТИРАННЯ СТОРІНКИ (Перед записом у Flash обов'язково треба стерти сторінку)
        FLASH_EraseInitTypeDef eraseStruct = {};
        eraseStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
        eraseStruct.PageAddress = FLASH_PAGE_ADDR;
        eraseStruct.NbPages     = 1;
        uint32_t pageError = 0;

        if (HAL_FLASHEx_Erase(&eraseStruct, &pageError) != HAL_OK) {
            HAL_FLASH_Lock();
            return false;
        }

        // 3. ЗАПИС ДАНИХ (по 32 біти / 1 Слово)
        for (uint32_t i = 0; i < wordsToWrite; i++) {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_PAGE_ADDR + (i * 4), dataPtr[i]) != HAL_OK) {
                HAL_FLASH_Lock();
                return false;
            }
        }

        // 4. БЛОКУВАННЯ FLASH
        HAL_FLASH_Lock();
        return true;
    }

} // namespace Hephaestus
