#pragma once

#include <vob/aoe/common/input/physical/Switch.h>

#include <vob/sta/enum_map.h>

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <array>
#include <string_view>

namespace vob::aoe::common
{
    struct Gamepad
    {
        enum class Button
        {
            Unknown = -1,

            // Default | Xbox
            A = 0,
            B,
            X,
            Y,
            LB,
            RB,
            Back,
            Start,
            Guide,
            LS,
            RS,
            Up,
            Right,
            Down,
            Left,
            Count,

            // Playstation
            Cross = A,
            Circle = B,
            Square = X,
            Triangle = Y,
            L1 = LB,
            L3 = LS,
            R1 = RB,
            R3 = RS,
            Select = Back,

            // Verbose
            LeftBumper = LB,
            RightBumper = RB,
            LeftThumb = LS,
            RightThumb = RS,
            DpadUp = Up,
            DpadDown = Down,
            DpadLeft = Left,
            DpadRight = Right
        };

        enum class Axis
        {
            Unknown = -1,

            // Default | Xbox
            LX = 0,
            LY,
            RX,
            RY,
            LT,
            RT,
            Count,

            // Playstation
            L2 = LT,
            R2 = RT,

            // Verbose
            LeftX = LX,
            LeftY = LY,
            RightX = RX,
            RightY = RY
        };

        Switch m_state{};
        std::string_view m_name{};
        sta::enum_map<Button, Button::A, Button::Count, Switch> m_buttons{};
        sta::enum_map<Axis, Axis::LX, Axis::Count, float> m_axes{};
    };

    inline Gamepad::Button gamepadButtonFromGlfw(int a_glfwGamepadButtonId)
    {
        if (a_glfwGamepadButtonId < GLFW_GAMEPAD_BUTTON_A || a_glfwGamepadButtonId > GLFW_GAMEPAD_BUTTON_LAST)
        {
            return Gamepad::Button::Unknown;
        }

        return static_cast<Gamepad::Button>(a_glfwGamepadButtonId);
    }

    inline int gamepadButtonToGlfw(Gamepad::Button a_gamepadButton)
    {
        return static_cast<int>(a_gamepadButton);
    }

    inline Gamepad::Axis gamepadAxisFromGlfw(int a_glfwGamepadAxisId)
    {
        if (a_glfwGamepadAxisId < GLFW_GAMEPAD_AXIS_LEFT_X || a_glfwGamepadAxisId > GLFW_GAMEPAD_AXIS_LAST)
        {
            return Gamepad::Axis::Unknown;
        }

        return static_cast<Gamepad::Axis>(a_glfwGamepadAxisId);
    }

    inline int gamepadAxisToGlfw(Gamepad::Axis a_gamepadAxis)
    {
        return static_cast<int>(a_gamepadAxis);
    }
}