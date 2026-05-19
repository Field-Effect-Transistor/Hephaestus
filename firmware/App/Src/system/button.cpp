#include "system/button.hpp"

namespace Hephaestus {

    ButtonEvent Button::update(bool is_pressed, uint32_t tick_ms) {
        ButtonEvent event = ButtonEvent::None;
        uint32_t elapsed = tick_ms - _lastStateChangeTime;

        switch (_state) {

            case State::Idle:
                if (is_pressed) {
                    _state = State::DebouncePress;
                    _lastStateChangeTime = tick_ms;
                }
                break;

            case State::DebouncePress:
                if (elapsed >= _debounceTimeMs) {
                    if (is_pressed) {
                        _state = State::Pressed;
                        _lastStateChangeTime = tick_ms;
                        event = ButtonEvent::Pressed;
                    } else {
                        _state = State::Idle;
                    }
                }
                break;

            case State::Pressed:
                if (!is_pressed) {
                    _state = State::WaitDouble;
                    _lastStateChangeTime = tick_ms;
                } else if (elapsed >= _longPressMs) {
                    _state = State::Hold;
                    _lastRepeatTime = tick_ms;
                    event = ButtonEvent::LongPress;
                }
                break;

            case State::WaitDouble:
                if (is_pressed) {
                    _state = State::DebounceSecondPress;
                    _lastStateChangeTime = tick_ms;
                } else if (elapsed >= _doubleClickWaitMs) {
                    _state = State::Idle;
                    event = ButtonEvent::SingleClick;
                }
                break;

            case State::DebounceSecondPress:
                if (elapsed >= _debounceTimeMs) {
                    if (is_pressed) {
                        _state = State::ConsumeRelease;
                        event = ButtonEvent::DoubleClick;
                    } else {
                        _state = State::WaitDouble;
                        _lastStateChangeTime = tick_ms;
                    }
                }
                break;

            case State::ConsumeRelease:
                if (!is_pressed) {
                    _state = State::Idle;
                }
                break;

            case State::Hold:
                if (!is_pressed) {
                    _state = State::DebounceRelease;
                    _lastStateChangeTime = tick_ms;
                } else if ((tick_ms - _lastRepeatTime) >= _longPressRepeatMs) {
                    _lastRepeatTime = tick_ms;
                    event = ButtonEvent::LongPressRepeat;
                }
                break;

            case State::DebounceRelease:
                if (elapsed >= _debounceTimeMs) {
                    if (!is_pressed) {
                        _state = State::Idle;
                        event = ButtonEvent::Released;
                    } else {
                        _state = State::Hold;
                        _lastRepeatTime = tick_ms;
                    }
                }
                break;
        }

        return event;
    }

} // namespace Hephaestus
