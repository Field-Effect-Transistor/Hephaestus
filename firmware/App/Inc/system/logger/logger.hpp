#pragma once

#ifndef PC_SIMULATOR
    #include "main.h"
#else
    #include <cstdint>
    extern "C" uint32_t HAL_GetTick();
#endif

#include "FreeRTOS.h"
#include "queue.h"

#include "interfaces/ILogSink.hpp"
#include <cstdio>
#include <vector>

namespace Hephaestus {

    enum class LogLevel { Debug, Info, Warn, Error, Fatal, None };

    constexpr LogLevel BUILD_LOG_LEVEL = LogLevel::Debug;
    constexpr size_t MAX_LOG_MESSAGE_LEN = 96;

    struct LogMessage {
        char buffer[80];
    };

    class Logger {
    private:
        static QueueHandle_t logQueue;
        static std::vector<ILogSink*> sinks;
        static LogLevel runtimeLogLevel;

        template<LogLevel L, typename... Args>
        static void logInternal(const char* tag, const char* format, Args... args) {
            if constexpr (L >= BUILD_LOG_LEVEL) {
                if (L >= runtimeLogLevel && logQueue != nullptr) {
                    LogMessage msg;
                    uint32_t timeMs = HAL_GetTick(); 
                    
                    const char* levelStr = getLevelString(L);

                    int offset = snprintf(msg.buffer, sizeof(msg.buffer), "[%6lu] [%s] [%s] ", timeMs, levelStr, tag);
                    
                    if (offset > 0 && offset < (int)sizeof(msg.buffer)) {
                        int bodyLen = snprintf(msg.buffer + offset, sizeof(msg.buffer) - offset, format, args...);
                        if (bodyLen > 0) {
                            offset += bodyLen;
                        }
                    }
                    
                    if (offset >= (int)sizeof(msg.buffer) - 2) {
                        offset = sizeof(msg.buffer) - 3;
                    }
                    msg.buffer[offset] = '\r';
                    msg.buffer[offset + 1] = '\n';
                    msg.buffer[offset + 2] = '\0';
                    
                    if (xPortIsInsideInterrupt()) {
                        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                        xQueueSendFromISR(logQueue, &msg, &xHigherPriorityTaskWoken);
                        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
                    } else {
                        xQueueSend(logQueue, &msg, 0); 
                    }
                }
            }
        }

        static const char* getLevelString(LogLevel l) {
            switch(l) {
                case LogLevel::Debug: return "DBG";
                case LogLevel::Info:  return "INF";
                case LogLevel::Warn:  return "WRN";
                case LogLevel::Error: return "ERR";
                case LogLevel::Fatal: return "FTL";
                default: return "UNK";
            }
        }

    public:
        static void init();
        static void setRuntimeLevel(LogLevel level) { runtimeLogLevel = level; }
        
        static void addSink(ILogSink* sink) {
            sinks.push_back(sink);
        }

        static void taskLoop(void* params);

        template<typename... Args> static void debug(const char* tag, const char* format, Args... args) { logInternal<LogLevel::Debug>(tag, format, args...); }
        template<typename... Args> static void info(const char* tag, const char* format, Args... args)  { logInternal<LogLevel::Info>(tag, format, args...); }
        template<typename... Args> static void warn(const char* tag, const char* format, Args... args)  { logInternal<LogLevel::Warn>(tag, format, args...); }
        template<typename... Args> static void error(const char* tag, const char* format, Args... args) { logInternal<LogLevel::Error>(tag, format, args...); }
        template<typename... Args> static void fatal(const char* tag, const char* format, Args... args) { logInternal<LogLevel::Fatal>(tag, format, args...); }
    };

} // namespace Hephaestus
