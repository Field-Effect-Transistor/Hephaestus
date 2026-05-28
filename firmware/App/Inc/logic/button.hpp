#pragma once
#include <cstdint>

/*
╔═══════════════════════════════════════════════════════════════════════════╗
║                        BUTTON STATE MACHINE                               ║
╠═══════════════════════════════════════════════════════════════════════════╣
║                                                                           ║
║                            ┌─────────┐                                    ║
║                            │  Idle   │◄──────────────────────┐            ║
║                            └────┬────┘                       │            ║
║                                 │ pressed                    │            ║
║                                 ▼                            │            ║
║                         ┌───────────────┐                    │            ║
║              bounce ───►│ DebouncePress │                    │            ║
║                         └───────┬───────┘                    │            ║
║                                 │ ok  ★Pressed               │            ║
║                                 ▼                            │            ║
║                            ┌─────────┐                       │            ║
║               ┌────────────│ Pressed │────────────┐          │            ║
║               │ released   └─────────┘  ≥800ms    │          │            ║
║               ▼                             ▼  ★LongPress    │            ║
║         ┌────────────┐               ┌──────────┐            │            ║
║         │ WaitDouble │               │   Hold   │──┐         │            ║
║         └──────┬─────┘               └────┬─────┘  │≥100ms   │            ║
║    ≥250ms │    │ pressed                  │        │★Repeat  │            ║
║  ★Single  │    ▼                          │released└─────────┘            ║
║    Click  │  ┌──────────────────┐         ▼                               ║
║           │  │DebounceSecondPrs │   ┌─────────────────┐                   ║
║           │  └────────┬─────────┘   │ DebounceRelease │                   ║
║           │  ok│      │bounce       └────────┬────────┘                   ║
║           │   ★│      └──►WaitDouble  ok│     │bounce                     ║ 
║           │DoubleClick                  │★    └──►Hold                    ║
║           │    ▼                     Released                             ║
║           │  ┌────────────────┐         │                                 ║
║           │  │ ConsumeRelease │         │                                 ║
║           │  └───────┬────────┘         │                                 ║
║           │ released │                  │                                 ║
║           └──────────┴──────────────────┘                                 ║
║                               │                                           ║
║                               ▼                                           ║
║                            ┌─────────┐                                    ║
║                            │  Idle   │                                    ║
║                            └─────────┘                                    ║
║                                                                           ║
║  ★ = подія що генерується                                                 ║
╚═══════════════════════════════════════════════════════════════════════════╝
*/

namespace Hephaestus {

    enum class ButtonEvent {
        None,
        Pressed,
        Released,
        SingleClick,
        DoubleClick,
        LongPress,
        LongPressRepeat
    };

    class Button {
    private:
        enum class State {
            Idle,
            DebouncePress,
            Pressed,
            WaitDouble,
            DebounceSecondPress,
            ConsumeRelease,
            DebounceRelease,
            Hold
        };

        State    _state = State::Idle;
        uint32_t _lastStateChangeTime = 0;
        uint32_t _lastRepeatTime = 0;

        const uint32_t _debounceTimeMs     = 20;
        const uint32_t _doubleClickWaitMs  = 250;
        const uint32_t _longPressMs        = 800;
        const uint32_t _longPressRepeatMs  = 100;

    public:
        Button() = default;
        ButtonEvent update(bool is_pressed, uint32_t tick_ms);
    };

} // namespace Hephaestus