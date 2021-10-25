#include <vob/aoe/common/window/GlfwInputSystem.h>


using namespace vob::aoe::common;


namespace
{
	inline void processEvent(KeyEvent const& a_keyEvent, InputComponent& a_worldInput)
	{
		if (a_keyEvent.m_action == KeyEvent::Action::Repeat)
		{
			return;
		}

		auto const key = toKey(a_keyEvent.m_keyCode);
		if (key != Keyboard::Key::Unknown)
		{
			auto& keyState = a_worldInput.m_keyboard.m_keys[key];
			keyState.m_changed = true;
			keyState.m_isActive = a_keyEvent.m_action == KeyEvent::Action::Press;
		}
	}

	inline void processEvent(MouseMoveEvent const& a_mouseMoveEvent, InputComponent& a_worldInput)
	{
		a_worldInput.m_mouse.m_move +=
			vob::aoe::vec2{ a_mouseMoveEvent.m_position } - a_worldInput.m_mouse.m_position;
		a_worldInput.m_mouse.m_position = a_mouseMoveEvent.m_position;
	}

	inline void processEvent(MouseButtonEvent const& a_mouseButtonEvent, InputComponent& a_worldInput)
	{
		auto const button = toButton(a_mouseButtonEvent.m_button);
		if (button != Mouse::Button::Unknown)
		{
			auto& buttonState = a_worldInput.m_mouse.m_buttons[button];
			buttonState.m_changed = true;
			buttonState.m_isActive = a_mouseButtonEvent.m_pressed;
		}
	}

	inline void processEvent(MouseScrollEvent const& a_mouseScrollEvent, InputComponent& a_textInput)
	{
		// TODO
	}

	inline void processEvent(TextEvent const& a_textEvent, InputComponent& a_textInput)
	{
		// TODO?
	}

	inline void processEvent(MouseEnterEvent const& a_mouseEnterEvent, InputComponent& a_textInput)
	{}

	inline void resetFrameState(InputComponent& a_worldInput)
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

	inline void processEvents(IWindow& a_window, InputComponent& a_worldInput)
	{
		a_window.pollEvents();
		for (auto const& polledEvent : a_window.getPolledEvents())
		{
			std::visit([&a_worldInput](auto const& a_event) {
				processEvent(a_event, a_worldInput);
			}, polledEvent);
		}
	}

	inline void updateHoverState(IWindow& a_window, InputComponent& a_worldInput)
	{
		auto isActive = a_window.isHovered();
		auto wasActive = a_worldInput.m_mouse.m_hover.m_isActive;
		a_worldInput.m_mouse.m_hover.m_changed = isActive != wasActive;
		a_worldInput.m_mouse.m_hover.m_isActive = isActive;
	}
}

GlfwInputSystem::GlfwInputSystem(ecs::WorldDataProvider& a_wdp)
	: m_worldWindowComponent{ *a_wdp.getWorldComponent<WindowComponent>() }
	, m_worldInput{ *a_wdp.getWorldComponent<InputComponent>() }
	, m_worldStop{ a_wdp.getStopBool() }
{}

void GlfwInputSystem::update() const
{
	auto& window = m_worldWindowComponent.getWindow();

	resetFrameState(m_worldInput);

	if (window.shouldClose())
	{
		m_worldStop = true;
	}

	processEvents(window, m_worldInput);

	updateHoverState(window, m_worldInput);
}
