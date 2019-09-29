#pragma once

#include "aoe/core/ecs/WorldDataProvider.h"
#include "WindowComponent.h"
#include <SFML/Window/Event.hpp>
#include "InputComponent.h"
#include "CursorComponent.h"
#include <iomanip>
#include <aoe/common/imgui/imgui_sfml.h>

namespace aoe
{
	namespace common
	{
		class InputSystem
		{
		public:
			explicit InputSystem(ecs::WorldDataProvider& a_wdp)
				: m_worldWindow{ *a_wdp.getWorldComponent<WindowComponent>() }
				, m_worldInput{ *a_wdp.getWorldComponent<InputComponent>() }
				, m_worldCursor{ *a_wdp.getWorldComponent<CursorComponent>() }
			{}

			void update() const
			{
				for (auto& key : m_worldInput.m_keyboard.m_keys)
				{
					key.m_changed = false;
				}

				for (auto& button : m_worldInput.m_mouse.m_buttons)
				{
					button.m_changed = false;
				}

				// TODO maybe go event ?
				m_worldInput.m_mouse.m_move = sf::Vector2i{};
				auto const mousePosition = sf::Mouse::getPosition(m_worldWindow.m_window);
				m_worldInput.m_mouse.m_move = mousePosition;
				m_worldInput.m_mouse.m_move -= m_worldInput.m_mouse.m_position;
				m_worldInput.m_mouse.m_position = mousePosition;

				sf::Event event{};
				while (m_worldWindow.getWindow().pollEvent(event))
				{
					// ImGui::SFML::ProcessEvent(event);
					switch (event.type)
					{
					case sf::Event::MouseButtonPressed:
					{
						auto& but = m_worldInput.m_mouse.m_buttons[event.mouseButton.button];
						but.m_changed = true;
						but.m_pressed = true;
						break;

					}
					case sf::Event::MouseButtonReleased:
					{
						auto& but = m_worldInput.m_mouse.m_buttons[event.mouseButton.button];
						but.m_changed = true;
						but.m_pressed = false;
						break;
					}
					case sf::Event::KeyPressed:
					{
						auto& key = m_worldInput.m_keyboard.m_keys[event.key.code];
						key.m_changed = true;
						key.m_pressed = true;
						break;
					}
					case sf::Event::KeyReleased:
					{
						auto& key = m_worldInput.m_keyboard.m_keys[event.key.code];
						key.m_changed = true;
						key.m_pressed = false;
						break;
					}
					default:
						break;
					}
				}

				if (m_worldCursor.m_center)
				{
					sf::Vector2i centerPosition;
					centerPosition.x = m_worldWindow.getWindow().getSize().x / 2;
					centerPosition.y = m_worldWindow.getWindow().getSize().y / 2;
					sf::Mouse::setPosition(centerPosition, m_worldWindow.getWindow());
					m_worldInput.m_mouse.m_position = centerPosition;
				}
				m_worldWindow.getWindow().setMouseCursorVisible(m_worldCursor.m_visible);
			}

			WindowComponent& m_worldWindow;
			InputComponent& m_worldInput;
			CursorComponent const& m_worldCursor;
		};
	}
}
