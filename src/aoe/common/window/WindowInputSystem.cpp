#include <vob/aoe/common/window/WindowInputSystem.h>

#include <vob/aoe/common/window/WorldWindowcomponent.h>
#include <vob/aoe/common/input/WorldInputcomponent.h>
#include <vob/aoe/common/input/KeyboardUtil.h>

#include <vob/aoe/ecs/WorldDataProvider.h>

#include <vob/misc/std/enum_traits.h>
#include <iostream>

using namespace vob::aoe::common;

namespace
{
	inline void processEvent(KeyEvent const& a_keyEvent, WorldInputComponent& a_worldInput)
	{
		if (a_keyEvent.m_action == KeyEvent::Action::Repeat)
		{
			return;
		}

		auto const key = KeyboardUtil::keyFromGlfw(a_keyEvent.m_keyCode);
		if (key != Keyboard::Key::Unknown)
		{
			std::cout << vob::mistd::enum_traits<Keyboard::Key>::cast(key).value_or("") << std::endl;
			auto& keyState = a_worldInput.m_keyboard.m_keys[key];
			keyState.m_changed = true;
			keyState.m_isActive = a_keyEvent.m_action == KeyEvent::Action::Press;
		}
	}

	inline void processEvent(MouseMoveEvent const& a_mouseMoveEvent, WorldInputComponent& a_worldInput)
	{
		a_worldInput.m_mouse.m_move +=
			glm::vec2{ a_mouseMoveEvent.m_position } - a_worldInput.m_mouse.m_position;
		a_worldInput.m_mouse.m_position = a_mouseMoveEvent.m_position;
	}

	inline void processEvent(MouseButtonEvent const& a_mouseButtonEvent, WorldInputComponent& a_worldInput)
	{
		auto const button = mouseButtonFromGlfw(a_mouseButtonEvent.m_button);
		if (button != Mouse::Button::Unknown)
		{
			auto& buttonState = a_worldInput.m_mouse.m_buttons[button];
			buttonState.m_changed = true;
			buttonState.m_isActive = a_mouseButtonEvent.m_pressed;
		}
	}

	inline void processEvent(MouseScrollEvent const& a_mouseScrollEvent, WorldInputComponent& a_textInput)
	{
		// TODO
	}

	inline void processEvent(TextEvent const& a_textEvent, WorldInputComponent& a_textInput)
	{
		// TODO?
	}

	inline void processEvent(MouseEnterEvent const& a_mouseEnterEvent, WorldInputComponent& a_textInput)
	{}

	inline void resetFrameState(WorldInputComponent& a_worldInput)
	{
		a_worldInput.m_mouse.m_move = {};

		for (auto& key : a_worldInput.m_keyboard.m_keys)
		{
			key.m_changed = false;
		}

		for (auto& button : a_worldInput.m_mouse.m_buttons)
		{
			button.m_changed = false;
		}
	}

	inline void processEvents(IWindow& a_window, WorldInputComponent& a_worldInput)
	{
		a_window.pollEvents();
		for (auto const& polledEvent : a_window.getPolledEvents())
		{
			std::visit([&a_worldInput](auto const& a_event) {
				processEvent(a_event, a_worldInput);
			}, polledEvent);
		}
	}

	inline void updateHoverState(IWindow& a_window, WorldInputComponent& a_worldInput)
	{
		auto isActive = a_window.isHovered();
		auto wasActive = a_worldInput.m_mouse.m_hover.m_isActive;
		a_worldInput.m_mouse.m_hover.m_changed = isActive != wasActive;
		a_worldInput.m_mouse.m_hover.m_isActive = isActive;
	}

	inline void updateGamepad(std::size_t a_gamepadIndex, Gamepad& a_gamepad)
	{
		if (!glfwJoystickPresent(static_cast<int>(a_gamepadIndex)))
		{
			if (a_gamepad.m_state.m_isActive)
			{
				a_gamepad.m_name = {};
			}
			a_gamepad.m_state.update(false);
			return;
		}

		if (!glfwJoystickIsGamepad(static_cast<int>(a_gamepadIndex)))
		{
			if (a_gamepad.m_state.m_isActive)
			{
				a_gamepad.m_name = {};
			}
			a_gamepad.m_state.update(false);
			return;
		}

		GLFWgamepadstate state;
		if (glfwGetGamepadState(static_cast<int>(a_gamepadIndex), &state) == GLFW_FALSE)
		{
			if (a_gamepad.m_state.m_isActive)
			{
				a_gamepad.m_name = {};
			}
			a_gamepad.m_state.update(false);
			return;
		}

		if (!a_gamepad.m_state.m_isActive)
		{
			a_gamepad.m_name = glfwGetGamepadName(static_cast<int>(a_gamepadIndex));
		}
		a_gamepad.m_state.update(true);

		for (auto i = 0; i != a_gamepad.m_buttons.size(); ++i)
		{
			auto button = Gamepad::Button(a_gamepad.m_buttons.begin_value + i);
			auto isActive = state.buttons[gamepadButtonToGlfw(button)] == GLFW_PRESS;
			auto& buttonState = a_gamepad.m_buttons[i];
			buttonState.m_changed = buttonState.m_isActive != isActive;
			buttonState.m_isActive = isActive;
		}

		for (auto i = 0; i != a_gamepad.m_axes.size(); ++i)
		{
			auto axis = Gamepad::Axis{ a_gamepad.m_axes.begin_value + i };
			auto& axisState = a_gamepad.m_axes[i];
			axisState = state.axes[gamepadAxisToGlfw(axis)];
		}
	}

	inline void updateGamepads(WorldInputComponent& a_worldInput)
	{
		for (auto i = 0; i < a_worldInput.m_gamepads.size(); ++i)
		{
			updateGamepad(i, a_worldInput.m_gamepads[i]);
		}
	}
}

WindowInputSystem::WindowInputSystem(aoecs::WorldDataProvider& a_wdp)
	: m_worldWindowComponent{ *a_wdp.getWorldComponent<WorldWindowComponent>() }
	, m_worldInputComponent{ *a_wdp.getWorldComponent<WorldInputComponent>() }
	, m_stopManager{ a_wdp.getStopManager() }
{}

void WindowInputSystem::update() const
{
	auto& window = m_worldWindowComponent.getWindow();

	resetFrameState(m_worldInputComponent);

	if (window.shouldClose())
	{
		m_stopManager.set_should_stop(true);
	}

	processEvents(window, m_worldInputComponent);

	updateHoverState(window, m_worldInputComponent);

	updateGamepads(m_worldInputComponent);
}
