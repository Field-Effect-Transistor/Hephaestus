// App/Src/platform/pc/file_storage.cpp
#include "platform/pc/file_storage.hpp"

#include <cstdio>
#include <iostream>

namespace Hephaestus {

    bool FileStorage::load(SystemConfig& config) {
        FILE* f = fopen(FILENAME, "rb");
        if (!f) return false;

        size_t readSize = fread(&config, 1, sizeof(SystemConfig), f);
        fclose(f);

        return (readSize == sizeof(SystemConfig));
    }

    bool FileStorage::save(const SystemConfig& config) {
        FILE* f = fopen(FILENAME, "wb");
        if (!f) {
            std::cerr << "Failed to open config file for writing!\n";
            return false;
        }

        size_t written = fwrite(&config, 1, sizeof(SystemConfig), f);
        fclose(f);

        return (written == sizeof(SystemConfig));
    }

} // namespace Hephaestus
