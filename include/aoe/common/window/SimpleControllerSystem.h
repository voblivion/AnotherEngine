#pragma once
#include <aoe/core/ecs/WorldDataProvider.h>
#include <aoe/common/space/VelocityComponent.h>
#include <aoe/common/window/SimpleControllerComponent.h>
#include <aoe/common/window/InputComponent.h>
#include <aoe/common/space/TransformComponent.h>
#include <aoe/common/window/CursorComponent.h>
#include <aoe/common/time/TimeComponent.h>
#include <glm/gtc/quaternion.hpp>
#include <cmath>
#include <algorithm>

//todo
template <typename T>
bool equalWithEpsilon(T lhs, T rhs, T epsilon = std::numeric_limits<T>::epsilon())
{
	return std::abs(lhs - rhs) < epsilon;
}

namespace aoe
{
	namespace common
	{
		class SimpleControllerSystem
		{
			using Components = ecs::ComponentTypeList<TransformComponent
				, VelocityComponent
				, SimpleControllerComponent>;
		public:
			explicit SimpleControllerSystem(ecs::WorldDataProvider& a_worldDataProvider)
				: m_worldInput{ *a_worldDataProvider.getWorldComponent<InputComponent const>() }
				, m_worldCursor{ *a_worldDataProvider.getWorldComponent<CursorComponent>() }
				, m_worldTime{ *a_worldDataProvider.getWorldComponent<TimeComponent const>() }
				, m_entities{ a_worldDataProvider.getEntityList(Components{}) }
			{}


			static glm::vec3 toVec3(glm::vec4 const v)
			{
				return glm::vec3{ v.x, v.y, v.z };
			}

			void update() const
			{
				for (auto const& entity : m_entities)
				{
					auto& transform = entity.getComponent<TransformComponent>();
					auto& velocity = entity.getComponent<VelocityComponent>();
					auto& simpleController = entity.getComponent<SimpleControllerComponent>();

					// Compute local required movement
					auto linearSpeed{ 4.0f };
					glm::vec3 localMove{ 0.0f };
					localMove.x -= m_worldInput.m_keyboard.m_keys[sf::Keyboard::S];
					localMove.x += m_worldInput.m_keyboard.m_keys[sf::Keyboard::F];
					localMove.z -= m_worldInput.m_keyboard.m_keys[sf::Keyboard::E];
					localMove.z += m_worldInput.m_keyboard.m_keys[sf::Keyboard::D];
					localMove.y -= m_worldInput.m_keyboard.m_keys[sf::Keyboard::V];
					localMove.y += m_worldInput.m_keyboard.m_keys[sf::Keyboard::Space];
					linearSpeed += m_worldInput.m_keyboard.m_keys[sf::Keyboard::A] * 4;
					auto const directionLength = glm::length(localMove);
					if (directionLength > FLT_EPSILON)
					{
						localMove /= directionLength;
					}

					// Make local required movement global velocity
					velocity.m_linear = glm::vec3{ glm::mat4_cast(transform.m_rotation)
						* glm::vec4{ localMove, 1.0f } * linearSpeed };

					if (m_worldInput.m_mouse.m_buttons[sf::Mouse::Right])
					{
						if (m_worldInput.m_mouse.m_buttons[sf::Mouse::Right].m_changed)
						{
							m_worldCursor.m_visible = false;
							m_worldCursor.m_center = true;
						}

						// Compute new orientation
						auto& euler = simpleController.m_orientation;
						euler.y += m_worldTime.m_elapsedTime
							* -m_worldInput.m_mouse.m_move.x;
						euler.y = std::fmod(euler.y, 2 * glm::pi<float>());

						euler.x += m_worldTime.m_elapsedTime
							* -m_worldInput.m_mouse.m_move.y;
						euler.x = std::clamp(euler.x
							, -glm::pi<float>() / 2
							, glm::pi<float>() / 2);

						transform.m_rotation = glm::quat{ euler };
					}
					else
					{
						if (m_worldInput.m_mouse.m_buttons[sf::Mouse::Right].m_changed)
						{
							m_worldCursor.m_visible = true;
							m_worldCursor.m_center = false;
						}
					}
				}
			}

		private:
			InputComponent const& m_worldInput;
			CursorComponent& m_worldCursor;
			TimeComponent const& m_worldTime;
			ecs::SystemEntityList<TransformComponent, VelocityComponent
				, SimpleControllerComponent> const& m_entities;
		};
	}
}
