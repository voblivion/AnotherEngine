#pragma once

#include <vob/aoe/common/input/physical/Gamepad.h>
#include <vob/aoe/common/input/physical/Keyboard.h>
#include <vob/aoe/common/input/physical/Mouse.h>

#include <vob/aoe/common/input/WorldInputcomponent.h>

#include <variant>


namespace vob::aoe::common
{
    struct PhysicalSwitchHandle_KeyboardKey
    {
        Keyboard::Key m_key;
    };

    struct PhysicalSwitchHandle_MouseButton
    {
        Mouse::Button m_button;
    };

    struct PhysicalSwitchHandle_GamepadButton
    {
        std::size_t m_gamepadId = -1u;
        Gamepad::Button m_button = Gamepad::Button::Unknown;
    };

    using PhysicalSwitchHandle = std::variant<
        PhysicalSwitchHandle_KeyboardKey
        , PhysicalSwitchHandle_MouseButton
        , PhysicalSwitchHandle_GamepadButton
    >;
}