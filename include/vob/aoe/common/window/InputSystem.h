#pragma once

#include <vob/aoe/core/ecs/WorldDataProvider.h>

#include <vob/aoe/common/render/IWindow.h>
#include <vob/aoe/common/window/CursorComponent.h>
#include <vob/aoe/common/window/InputComponent.h>
#include <vob/aoe/common/window/WindowComponent.h>

#include <iomanip>



namespace vob::aoe::common
{
	class InputSystem
	{
	public:
		explicit InputSystem(ecs::WorldDataProvider& a_wdp)
			: m_worldWindowComponent{ *a_wdp.getWorldComponent<WindowComponent>() }
			, m_worldInput{ *a_wdp.getWorldComponent<InputComponent>() }
			, m_worldCursor{ *a_wdp.getWorldComponent<CursorComponent>() }
			, m_worldStop{ a_wdp.getStopBool() }
		{
			
		}

		void update() const
		{
			auto& window = m_worldWindowComponent.getWindow();

			if (window.shouldClose())
			{
				m_worldStop = true;
			}

			auto mouseMoved = false;
			window.pollEvents();
			for (auto const& polledEvent : window.getPolledEvents())
			{
				std::visit([this, &mouseMoved](auto const& a_event)
				{
					using EventType = std::decay_t<decltype(a_event)>;
					if constexpr (std::is_same_v<EventType, common::KeyEvent>)
					{
						auto const& keyEvent = static_cast<common::KeyEvent const&>(a_event);

						if (keyEvent.m_action == common::KeyEvent::Action::Repeat)
							return;

						auto const key = static_cast<std::size_t>(toKey(keyEvent.m_keyCode));
						if (key < m_worldInput.m_keyboard.m_keys.size())
						{
							auto& keyState = m_worldInput.m_keyboard.m_keys[key];
							keyState.m_changed = true;
							keyState.m_pressed = keyEvent.m_action == common::KeyEvent::Action::Press;
						}
					}
					else if constexpr (std::is_same_v<EventType, common::MouseMoveEvent>)
					{
						auto const& mouseMoveEvent = static_cast<common::MouseMoveEvent const&>(a_event);
						m_worldInput.m_mouse.m_move = mouseMoveEvent.m_position;
						m_worldInput.m_mouse.m_move -= m_worldInput.m_mouse.m_position;
						m_worldInput.m_mouse.m_position = mouseMoveEvent.m_position;
						mouseMoved = true;
					}
					else if constexpr (std::is_same_v<EventType, common::MouseButtonEvent>)
					{
						auto const& mouseButtonEvent = static_cast<common::MouseButtonEvent const&>(a_event);
						auto& buttonState = m_worldInput.m_mouse.m_buttons[mouseButtonEvent.m_button];
						buttonState.m_changed = true;
						buttonState.m_pressed = mouseButtonEvent.m_pressed;
					}
				}, polledEvent);
			}
			m_worldInput.m_mouse.m_moving.m_changed = !m_worldInput.m_mouse.m_moving.m_pressed;
			if (mouseMoved)
			{
				m_worldInput.m_mouse.m_moving.m_pressed = true;
			}
			else
			{
				m_worldInput.m_mouse.m_moving.m_pressed = false;
				m_worldInput.m_mouse.m_move = {};
			}
			// TODO : Temporary because at start we don't know
			m_worldInput.m_mouse.m_inside.m_pressed = window.isHovered();

			window.setCursorState(m_worldCursor.m_state);
		}

		WindowComponent& m_worldWindowComponent;
		InputComponent& m_worldInput;
		CursorComponent const& m_worldCursor;
		bool& m_worldStop;
	};
}
