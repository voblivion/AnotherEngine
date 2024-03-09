#include <vob/aoe/input/reference.h>


namespace vob::aoein
{
	namespace detail
	{
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

	switch_reference::switch_reference(mouse::button const a_button)
		: m_variantRef{ a_button }
	{}

	switch_reference::switch_reference(keyboard::key const a_key)
		: m_variantRef{ a_key }
	{}

	switch_reference::switch_reference(gamepad_index const a_gamepadIndex, gamepad::button const a_gamepadButton)
		: m_variantRef{ gamepad_index_button{ a_gamepadIndex, a_gamepadButton } }
	{}

	bool switch_reference::is_pressed(inputs const& a_inputs)
	{
		return std::visit(
			[&a_inputs](auto const& a_variantRef) {
				return detail::get_switch(a_inputs, a_variantRef).is_pressed();
			}, m_variantRef);
	}

	bool switch_reference::has_changed(inputs const& a_inputs)
	{
		return std::visit(
			[&a_inputs](auto const& a_variantRef) {
				return detail::get_switch(a_inputs, a_variantRef).has_changed();
			}, m_variantRef);
	}

	axis_reference::axis_reference(mouse::axis const a_mouseAxis)
		: m_variantRef{ a_mouseAxis }
	{}

	axis_reference::axis_reference(gamepad_index const a_gamepadIndex, gamepad::axis const a_gamepadAxis)
		: m_variantRef{ gamepad_index_axis{ a_gamepadIndex, a_gamepadAxis } }
	{}

	float axis_reference::get_change(inputs const& a_inputs)
	{
		return std::visit(
			[&a_inputs](auto const& a_variantRef) {
				return detail::get_axis(a_inputs, a_variantRef).get_change();
			}, m_variantRef);
	}

	float axis_reference::get_value(inputs const& a_inputs)
	{
		return std::visit(
			[&a_inputs](auto const& a_variantRef) {
				return detail::get_axis(a_inputs, a_variantRef).get_value();
			}, m_variantRef);
	}
}
