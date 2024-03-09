#pragma once

#include <vob/aoe/api.h>
#include <vob/aoe/input/inputs.h>

#include <utility>
#include <variant>


namespace vob::aoein
{
	using gamepad_index_button = std::pair<gamepad_index, gamepad::button>;

	class VOB_AOE_API switch_reference
	{
	public:
		switch_reference(mouse::button const a_button);

		switch_reference(keyboard::key const a_key);

		switch_reference(gamepad_index const a_gamepadIndex, gamepad::button const a_gamepadButton);

		bool is_pressed(inputs const& a_inputs);

		bool has_changed(inputs const& a_inputs);

	private:
		std::variant<mouse::button, keyboard::key, gamepad_index_button> m_variantRef;
	};

	using gamepad_index_axis = std::pair<gamepad_index, gamepad::axis>;

	class VOB_AOE_API axis_reference
	{
	public:
		axis_reference(mouse::axis const a_mouseAxis);

		axis_reference(gamepad_index const a_gamepadIndex, gamepad::axis const a_gamepadAxis);

		float get_change(inputs const& a_inputs);

		float get_value(inputs const& a_inputs);

	private:
		std::variant<mouse::axis, gamepad_index_axis> m_variantRef;
	};
}
