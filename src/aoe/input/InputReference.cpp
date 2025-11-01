#include <vob/aoe/input/InputReference.h>


namespace vob::aoein
{
	namespace
	{
		SwitchInput const& getSwitch(
			Inputs const& a_inputs,
			Mouse::Button const& a_mouseButton)
		{
			return a_inputs.mouse.buttons[a_mouseButton];
		}

		SwitchInput const& getSwitch(
			Inputs const& a_inputs,
			Keyboard::Key const& a_keyboardKey)
		{
			return a_inputs.keyboard.keys[a_keyboardKey];
		}

		SwitchInput const& getSwitch(
			Inputs const& a_inputs,
			GamepadIndexButton const& a_gamepadIndexButton)
		{
			return a_inputs.gamepads[
				a_gamepadIndexButton.first].buttons[a_gamepadIndexButton.second];
		}

		AxisInput const& getAxis(
			Inputs const& a_inputs,
			Mouse::Axis const& a_mouseAxis)
		{
			return a_inputs.mouse.axes[a_mouseAxis];
		}

		AxisInput const& getAxis(
			Inputs const& a_inputs,
			GamepadIndexAxis const& a_gamepadIndexAxis)
		{
			return a_inputs.gamepads[a_gamepadIndexAxis.first].axes[a_gamepadIndexAxis.second];
		}
	}

	SwitchReference::SwitchReference(Mouse::Button const a_button)
		: m_variantRef{ a_button }
	{
	}

	SwitchReference::SwitchReference(Keyboard::Key const a_key)
		: m_variantRef{ a_key }
	{
	}

	SwitchReference::SwitchReference(int32_t const a_gamepadIndex, Gamepad::Button const a_gamepadButton)
		: m_variantRef{ GamepadIndexButton{ a_gamepadIndex, a_gamepadButton } }
	{
	}

	bool SwitchReference::isPressed(Inputs const& a_inputs)
	{
		return std::visit(
			[&a_inputs](auto const& a_variantRef) {
				return getSwitch(a_inputs, a_variantRef).isPressed();
			}, m_variantRef);
	}

	bool SwitchReference::hasChanged(Inputs const& a_inputs)
	{
		return std::visit(
			[&a_inputs](auto const& a_variantRef) {
				return getSwitch(a_inputs, a_variantRef).hasChanged();
			}, m_variantRef);
	}

	AxisReference::AxisReference(Mouse::Axis const a_mouseAxis)
		: m_variantRef{ a_mouseAxis }
	{
	}

	AxisReference::AxisReference(int32_t const a_gamepadIndex, Gamepad::Axis const a_gamepadAxis)
		: m_variantRef{ GamepadIndexAxis{ a_gamepadIndex, a_gamepadAxis } }
	{
	}

	float AxisReference::getChange(Inputs const& a_inputs)
	{
		return std::visit(
			[&a_inputs](auto const& a_variantRef) {
				return getAxis(a_inputs, a_variantRef).getChange();
			}, m_variantRef);
	}

	float AxisReference::getValue(Inputs const& a_inputs)
	{
		return std::visit(
			[&a_inputs](auto const& a_variantRef) {
				return getAxis(a_inputs, a_variantRef).getValue();
			}, m_variantRef);
	}
}
