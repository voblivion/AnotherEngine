#pragma once

#include <vob/aoe/common/input/physical/Keyboard.h>
#include <vob/aoe/common/input/physical/Mouse.h>

#include <variant>


namespace vob::aoe::common
{
    // TODO : handle controller buttons

    bool isActiveMapping(Keyboard::Key a_key, Keyboard const& a_keyboard, Mouse const& a_mouse)
    {
        return a_keyboard.m_keys[a_key].m_isActive;
    }

    bool isActiveMapping(Mouse::Button a_button, Keyboard const& a_keyboard, Mouse const& a_mouse)
    {
        return a_mouse.m_buttons[a_button].m_isActive;
    }

    bool hasChangedMapping(Keyboard::Key a_key, Keyboard const& a_keyboard, Mouse const& a_mouse)
    {
        return a_keyboard.m_keys[a_key].m_changed;
    }

    bool hasChangedMapping(Mouse::Button a_button, Keyboard const& a_keyboard, Mouse const& a_mouse)
    {
        return a_mouse.m_buttons[a_button].m_changed;
    }

    class PhysicalSwitch
    {
    public:
        bool isActive(Keyboard const& a_keyboard, Mouse const& a_mouse) const
        {
            return std::visit([&a_keyboard, &a_mouse](auto const& a_mapping) {
                return isActiveMapping(a_mapping, a_keyboard, a_mouse);
                }, m_mapping);
        }

        bool hasChanged(Keyboard const& a_keyboard, Mouse const& a_mouse) const
        {
            return std::visit([&a_keyboard, &a_mouse](auto const& a_mapping) {
                return hasChangedMapping(a_mapping, a_keyboard, a_mouse);
                }, m_mapping);
        }

    private:
        std::variant<Keyboard::Key, Mouse::Button> m_mapping;
    };
}