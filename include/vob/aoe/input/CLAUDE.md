# input (`vob::aoein`)

Translates raw device/window input (mouse, keyboard, gamepad) into logical game input: named axes and events, bound to physical device state.

- `Inputs.h` — raw per-device state (mouse/keyboard/gamepad).
- `InputReference.h`, `InputBindings.h`, `CommonInputBindings.h`, `InputBindingUtils.h`, `InputBinding.h` — binding types mapping device state/window events to axis/switch values.
- `GameInput.h`, `GameInputContext.h`, `GameInputBindingContext.h`, `GameInputUtils.h` — the logical layer: named values/events (`GameInputValueId`/`GameInputEventId`), registered and bound via the above.
- `WindowInputBindingSystem.h/.cpp` — systems updating bindings from window/device state each tick.
- `DebugGameInputContext`/`DebugGameInputSystem` — records input history for display.
