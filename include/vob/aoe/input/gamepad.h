#pragma once

#include <vob/aoe/input/switch_input.h>
#include <vob/aoe/input/axis_input.h>

#include <vob/misc/std/enum_map.h>

#include <array>
#include <string_view>

namespace vob::aoein
{
    struct gamepad
    {
        enum class button
        {
            unknown = -1,

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
            count,

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

        enum class axis
        {
            unknown = -1,

            // Default | Xbox
            LX = 0,
            LY,
            RX,
            RY,
            LT,
            RT,
            count,

            // Playstation
            L2 = LT,
            R2 = RT,

            // Verbose
            LeftX = LX,
            LeftY = LY,
            RightX = RX,
            RightY = RY
        };

        aoein::switch_input m_isConnected;
        std::string_view m_name{};
        mistd::enum_map<button, button::A, button::count, aoein::switch_input> m_buttons{};
        mistd::enum_map<axis, axis::LX, axis::count, aoein::axis_input> m_axes{};

        void update()
        {
            m_isConnected.update();
            for (auto& button : m_buttons)
            {
                button.update();
            }
        }
    };
}
