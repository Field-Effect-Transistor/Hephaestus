//  App/Src/system/logger/logger.cpp
#include "system/logger/logger.hpp"

namespace Hephaestus {

    QueueHandle_t Logger::logQueue = nullptr;
    std::vector<ILogSink*> Logger::sinks;
    LogLevel Logger::runtimeLogLevel = LogLevel::Debug;

    void Logger::init() {
        if (logQueue == nullptr) {
            logQueue = xQueueCreate(10, sizeof(LogMessage));
        }
    }

    void Logger::taskLoop(void* params) {
        (void)params;
        LogMessage msg;

        while (true) {
            if (xQueueReceive(logQueue, &msg, portMAX_DELAY) == pdTRUE) {
                
                size_t len = 0;
                while (len < sizeof(msg.buffer) && msg.buffer[len] != '\0') len++;

                for (auto sink : sinks) {
                    if (sink->isReady()) {
                        sink->write(reinterpret_cast<const uint8_t*>(msg.buffer), len);
                    }
                }
            }
        }
    }

} // namespace Hephaestus