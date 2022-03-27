#pragma once

#include <vob/aoe/input/physical_inputs.h>
#include <vob/aoe/input/gamepad.h>
#include <vob/aoe/input/keyboard.h>
#include <vob/aoe/input/mouse.h>

#include <variant>


namespace vob::aoein
{
	struct physical_switch_reference
	{
		physical_switch_reference(physical_inputs::gamepad_idx a_gamepadIndex, gamepad::button a_button)
			: m_physicalSwitch{ std::pair<physical_inputs::gamepad_idx, gamepad::button>{a_gamepadIndex, a_button} }
		{}

		physical_switch_reference(keyboard::key a_key)
			: m_physicalSwitch{ a_key }
		{}

		physical_switch_reference(mouse::button a_button)
			: m_physicalSwitch{ a_button }
		{}

		bool is_pressed(physical_inputs const& a_physicalInput)
		{
			return std::visit([this, &a_physicalInput](auto const a_switch) {
				return is_pressed(a_physicalInput, a_switch);
				}, m_physicalSwitch);
		}

		bool changed(physical_inputs const& a_physicalInput)
		{
			return std::visit([this, &a_physicalInput](auto const a_switch) {
				return changed(a_physicalInput, a_switch);
			}, m_physicalSwitch);
		}

	private:
		std::variant<
			std::pair<physical_inputs::gamepad_idx, gamepad::button>,
			keyboard::key,
			mouse::button> m_physicalSwitch;

		bool is_pressed(
			physical_inputs const& a_physicalInput,
			std::pair<physical_inputs::gamepad_idx, gamepad::button> a_gamepadButton)
		{
			if (a_gamepadButton.second == gamepad::button::unknown)
			{
				return false;
			}
			return a_physicalInput.m_gamepads[a_gamepadButton.first].m_buttons[a_gamepadButton.second].is_pressed();
		}

		bool is_pressed(physical_inputs const& a_physicalInput, keyboard::key a_keyboardKey)
		{
			if (a_keyboardKey == keyboard::key::unknown)
			{
				return false;
			}
			return a_physicalInput.m_keyboard.m_keys[a_keyboardKey].is_pressed();
		}

		bool is_pressed(physical_inputs const& a_physicalInput, mouse::button a_mouseButton)
		{
			if (a_mouseButton == mouse::button::unknown)
			{
				return false;
			}
			return a_physicalInput.m_mouse.m_buttons[a_mouseButton].is_pressed();
		}

		bool changed(
			physical_inputs const& a_physicalInput,
			std::pair<physical_inputs::gamepad_idx, gamepad::button> a_gamepadButton)
		{
			if (a_gamepadButton.second == gamepad::button::unknown)
			{
				return false;
			}
			return a_physicalInput.m_gamepads[a_gamepadButton.first].m_buttons[a_gamepadButton.second].changed();
		}

		bool changed(physical_inputs const& a_physicalInput, keyboard::key a_keyboardKey)
		{
			if (a_keyboardKey == keyboard::key::unknown)
			{
				return false;
			}
			return a_physicalInput.m_keyboard.m_keys[a_keyboardKey].changed();
		}

		bool changed(physical_inputs const& a_physicalInput, mouse::button a_mouseButton)
		{
			if (a_mouseButton == mouse::button::unknown)
			{
				return false;
			}
			return a_physicalInput.m_mouse.m_buttons[a_mouseButton].changed();
		}
	};
}
