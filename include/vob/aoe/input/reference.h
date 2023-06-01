#pragma once

#include <vob/aoe/input/inputs.h>

#include <utility>
#include <variant>


namespace vob::aoein
{
	namespace detail
	{
		using gamepad_index_button = std::pair<gamepad_index, gamepad::button>;
		
		switch_input const& get_switch(
			inputs const& a_inputs,
			mouse::button const& a_mouseButton)
		{
			return a_inputs.mouse.buttons[a_mouseButton];
		}

		switch_input const& get_switch(
			inputs const& a_inputs,
			keyboard::key const& a_keyboardKey)
		{
			return a_inputs.keyboard.keys[a_keyboardKey];
		}

		switch_input const& get_switch(
			inputs const& a_inputs,
			gamepad_index_button const& a_gamepadIndexButton)
		{
			return a_inputs.gamepads[
				a_gamepadIndexButton.first].buttons[a_gamepadIndexButton.second];
		}

		using gamepad_index_axis = std::pair<gamepad_index, gamepad::axis>;

		axis_input const& get_axis(
			inputs const& a_inputs,
			mouse::axis const& a_mouseAxis)
		{
			return a_inputs.mouse.axes[a_mouseAxis];
		}

		axis_input const& get_axis(
			inputs const& a_inputs,
			gamepad_index_axis const& a_gamepadIndexAxis)
		{
			return a_inputs.gamepads[a_gamepadIndexAxis.first].axes[a_gamepadIndexAxis.second];
		}
	}

	class switch_reference
	{
	public:
		switch_reference(mouse::button const a_button)
			: m_variantRef{ a_button }
		{}

		switch_reference(keyboard::key const a_key)
			: m_variantRef{ a_key }
		{}

		switch_reference(
			gamepad_index const a_gamepadIndex,
			gamepad::button const a_gamepadButton
		)
			: m_variantRef{ detail::gamepad_index_button{ a_gamepadIndex, a_gamepadButton } }
		{}

		bool is_pressed(inputs const& a_inputs)
		{
			return std::visit(
				[&a_inputs](auto const& a_variantRef) {
					return detail::get_switch(a_inputs, a_variantRef).is_pressed();
				}, m_variantRef);
		}

		bool has_changed(inputs const& a_inputs)
		{
			return std::visit(
				[&a_inputs](auto const& a_variantRef) {
					return detail::get_switch(a_inputs, a_variantRef).has_changed();
				}, m_variantRef);
		}

	private:
		std::variant<mouse::button, keyboard::key, detail::gamepad_index_button> m_variantRef;
	};

	class axis_reference
	{
	public:
		axis_reference(mouse::axis const a_mouseAxis)
			: m_variantRef{ a_mouseAxis }
		{}

		axis_reference(
			gamepad_index const a_gamepadIndex,
			gamepad::axis const a_gamepadAxis
		)
			: m_variantRef{ detail::gamepad_index_axis{ a_gamepadIndex, a_gamepadAxis } }
		{}

		float get_change(inputs const& a_inputs)
		{
			return std::visit(
				[&a_inputs](auto const& a_variantRef) {
					return detail::get_axis(a_inputs, a_variantRef).get_change();
				}, m_variantRef);
		}

		float get_value(inputs const& a_inputs)
		{
			return std::visit(
				[&a_inputs](auto const& a_variantRef) {
					return detail::get_axis(a_inputs, a_variantRef).get_value();
				}, m_variantRef);
		}

	private:
		std::variant<mouse::axis, detail::gamepad_index_axis> m_variantRef;
	};
}
