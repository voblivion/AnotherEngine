#include <vob/aoe/common/window/WindowInputSystem.h>

#include <vob/aoe/common/window/WorldWindowComponent.h>
#include <vob/aoe/common/input/WorldInputComponent.h>

#include <vob/aoe/core/ecs/WorldDataProvider.h>


using namespace vob::aoe::common;

namespace
{
	inline void processEvent(KeyEvent const& a_keyEvent, WorldInputComponent& a_worldInput)
	{
		if (a_keyEvent.m_action == KeyEvent::Action::Repeat)
		{
			return;
		}

		auto const key = keyFromGlfw(a_keyEvent.m_keyCode);
		if (key != Keyboard::Key::Unknown)
		{
			auto& keyState = a_worldInput.m_keyboard.m_keys[key];
			keyState.m_changed = true;
			keyState.m_isActive = a_keyEvent.m_action == KeyEvent::Action::Press;
		}
	}

	inline void processEvent(MouseMoveEvent const& a_mouseMoveEvent, WorldInputComponent& a_worldInput)
	{
		a_worldInput.m_mouse.m_move +=
			vob::aoe::vec2{ a_mouseMoveEvent.m_position } - a_worldInput.m_mouse.m_position;
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

		for (auto key : a_worldInput.m_keyboard.m_keys)
		{
			key.second.get().m_changed = false;
		}

		for (auto button : a_worldInput.m_mouse.m_buttons)
		{
			button.second.get().m_changed = false;
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

		for (auto buttonEntry : a_gamepad.m_buttons)
		{
			auto isActive = state.buttons[gamepadButtonToGlfw(buttonEntry.first)] == GLFW_PRESS;
			auto& button = buttonEntry.second.get();
			button.m_changed = button.m_isActive != isActive;
			button.m_isActive = isActive;
		}

		for (auto axisEntry : a_gamepad.m_axes)
		{
			axisEntry.second.get() = state.axes[gamepadAxisToGlfw(axisEntry.first)];
		}
	}

	inline void updateGamepads(WorldInputComponent& a_worldInput)
	{
		for (auto i = 0u; i < a_worldInput.m_gamepads.size(); ++i)
		{
			updateGamepad(i, a_worldInput.m_gamepads[i]);
		}
	}
}

WindowInputSystem::WindowInputSystem(ecs::WorldDataProvider& a_wdp)
	: m_worldWindowComponent{ *a_wdp.getWorldComponent<WorldWindowComponent>() }
	, m_worldInputComponent{ *a_wdp.getWorldComponent<WorldInputComponent>() }
	, m_worldStop{ a_wdp.getStopBool() }
{}

void WindowInputSystem::update() const
{
	auto& window = m_worldWindowComponent.getWindow();

	resetFrameState(m_worldInputComponent);

	if (window.shouldClose())
	{
		m_worldStop = true;
	}

	processEvents(window, m_worldInputComponent);

	updateHoverState(window, m_worldInputComponent);

	updateGamepads(m_worldInputComponent);
}
