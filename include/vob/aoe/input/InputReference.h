#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/input/Inputs.h>

#include <utility>
#include <variant>


namespace vob::aoein
{
	using GamepadIndexButton = std::pair<int32_t, Gamepad::Button>;

	class VOB_AOE_API SwitchReference
	{
	public:
		SwitchReference(Mouse::Button const a_button);

		SwitchReference(Keyboard::Key const a_key);

		SwitchReference(int32_t const a_gamepadIndex, Gamepad::Button const a_gamepadButton);

		bool isPressed(Inputs const& a_inputs);

		bool hasChanged(Inputs const& a_inputs);

	private:
		std::variant<Mouse::Button, Keyboard::Key, GamepadIndexButton> m_variantRef;
	};

	using GamepadIndexAxis = std::pair<int32_t, Gamepad::Axis>;

	class VOB_AOE_API AxisReference
	{
	public:
		AxisReference(Mouse::Axis const a_mouseAxis);

		AxisReference(int32_t const a_gamepadIndex, Gamepad::Axis const a_gamepadAxis);

		float getChange(Inputs const& a_inputs);

		float getValue(Inputs const& a_inputs);

	private:
		std::variant<Mouse::Axis, GamepadIndexAxis> m_variantRef;
	};
}
