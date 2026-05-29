// App/Inc/interfaces/IStorage.hpp
#pragma once

#include "system/system_config.hpp"

namespace Hephaestus {

    class IStorage {
    public:
        virtual ~IStorage() = default;

        virtual bool load(SystemConfig& config) = 0;

        virtual bool save(const SystemConfig& config) = 0;
    };

} // namespace Hephaestus