cmake_minimum_required(VERSION 3.22)

# --- Standard C++ Configuration ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_PROJECT_NAME Hephaestus)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE) # Enable clangd/IntelliSense support

# ==============================================================================
# BUILD CONFIGURATION
# ==============================================================================
option(BUILD_SIMULATOR "Build FreeRTOS PC Simulator instead of STM32 Firmware" OFF)

# Shared 3rd-party libraries
file(GLOB_RECURSE U8G2_SOURCES "${CMAKE_SOURCE_DIR}/Libs/u8g2/csrc/*.c")

if(BUILD_SIMULATOR)
    # --------------------------------------------------------------------------
    # TARGET: PC SIMULATOR (POSIX)
    # --------------------------------------------------------------------------
    project(${CMAKE_PROJECT_NAME} C CXX)
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE} (PC SIMULATOR)")

    add_compile_definitions(PC_SIMULATOR) # Trigger #ifdefs in C++ code

    add_executable(${CMAKE_PROJECT_NAME}_Sim
        # Platform Specifics (Mocks & PC Entry Point)
        App/Src/platform/pc/main_pc.cpp
        App/Src/platform/pc/hal_mock.cpp
        App/Src/platform/pc/sdl_context.cpp
        App/Src/platform/pc/file_storage.cpp 
        
        # Core Business Logic
        App/Src/logic/station_manager.cpp
        App/Src/logic/button.cpp
        App/Src/logic/heater_channel.cpp
        App/Src/logic/hot_air_channel.cpp
        
        # UI & Logging
        App/Src/ui/display_manager.cpp
        App/Src/system/logger/logger.cpp
        App/Src/ui/screens.cpp
        ${U8G2_SOURCES}

        # FreeRTOS Kernel & POSIX Port
        Middlewares/Third_Party/FreeRTOS/Source/list.c
        Middlewares/Third_Party/FreeRTOS/Source/queue.c
        Middlewares/Third_Party/FreeRTOS/Source/tasks.c
        Middlewares/Third_Party/FreeRTOS/Source/timers.c
        Libs/FreeRTOS_POSIX/heap_3.c
        Libs/FreeRTOS_POSIX/port.c
        Libs/FreeRTOS_POSIX/utils/wait_for_event.c
    )

    target_include_directories(${CMAKE_PROJECT_NAME}_Sim PRIVATE
        App/Inc/
        App/Src/platform/pc
        Middlewares/Third_Party/FreeRTOS/Source/include
        Libs/FreeRTOS_POSIX
        Libs/FreeRTOS_POSIX/utils
        ${CMAKE_SOURCE_DIR}/Libs/u8g2/csrc
    )

    # Link OS-level threads for FreeRTOS simulation
    find_package(Threads REQUIRED)
    find_package(SDL2 REQUIRED)
    target_include_directories(${CMAKE_PROJECT_NAME}_Sim PRIVATE ${SDL2_INCLUDE_DIRS})
    # -----------------------

    target_link_libraries(${CMAKE_PROJECT_NAME}_Sim 
        Threads::Threads 
        rt 
        ${SDL2_LIBRARIES}
    )

else()
    # --------------------------------------------------------------------------
    # TARGET: STM32 FIRMWARE
    # --------------------------------------------------------------------------
    project(${CMAKE_PROJECT_NAME} C CXX ASM)
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE} (STM32 FIRMWARE)")

    add_executable(${CMAKE_PROJECT_NAME})
    
    # Include CubeMX generated CMake target
    add_subdirectory(cmake/stm32cubemx)

    target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        # Platform Specifics (STM32 Hardware Adapters)
        App/Src/platform/stm32/app.cpp
        App/Src/platform/stm32/exti_registry.cpp
        App/Src/platform/stm32/adc_ads1115.cpp
        App/Src/platform/stm32/flash_storage.cpp
        App/Src/platform/stm32/encoder_tim.cpp
        App/Src/system/logger/uart_log_sink.cpp
        
        # Core Business Logic
        App/Src/logic/station_manager.cpp
        App/Src/logic/button.cpp
        App/Src/logic/heater_channel.cpp
        App/Src/logic/hot_air_channel.cpp
        
        # UI & Logging
        App/Src/ui/display_manager.cpp
        App/Src/system/logger/logger.cpp
        App/Src/ui/screens.cpp
        ${U8G2_SOURCES}
    )

    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        App/Inc/
        Core/Inc/
        
        # STM32 HAL & CMSIS
        Drivers/STM32F1xx_HAL_Driver/Inc
        Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
        Drivers/CMSIS/Device/ST/STM32F1xx/Include
        Drivers/CMSIS/Include

        # FreeRTOS ARM Port
        Middlewares/Third_Party/FreeRTOS/Source/include
        Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3
        Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2

        ${CMAKE_SOURCE_DIR}/Libs/u8g2/csrc
    )

    # Fix for CubeMX generated CMake (removes unnecessary libob.a dependency)
    list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)
    
    target_link_libraries(${CMAKE_PROJECT_NAME} stm32cubemx)

endif()
