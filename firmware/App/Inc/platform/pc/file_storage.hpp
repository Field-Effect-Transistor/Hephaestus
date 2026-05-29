// App/Inc/platform/pc/file_storage.hpp
#pragma once

#include "interfaces/IStorage.hpp"

namespace Hephaestus {

    class FileStorage : public IStorage {
    private:
        const char* FILENAME = "hephaestus_config.bin";

    public:
        FileStorage() = default;

        bool load(SystemConfig& config) override;
        bool save(const SystemConfig& config) override;
    };

} // namespace Hephaestus
