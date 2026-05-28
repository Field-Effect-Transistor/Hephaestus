//  App/Src/system/exti_regisry.cpp
#include "platform/stm32/exti_registry.hpp"

//  function override
extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_pin) {
    Hephaestus::EXTIRegistry::dispatch(GPIO_pin);
}
