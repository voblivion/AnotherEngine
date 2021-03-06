#pragma once
#include <vob/aoe/core/ecs/WorldDataProvider.h>
#include <vob/aoe/common/space/VelocityComponent.h>
#include <vob/aoe/common/window/SimpleControllerComponent.h>
#include <vob/aoe/common/window/InputComponent.h>
#include <vob/aoe/common/space/TransformComponent.h>
#include <vob/aoe/common/window/CursorComponent.h>
#include <vob/aoe/common/time/TimeComponent.h>
#include <glm/gtc/quaternion.hpp>
#include <cmath>
#include <algorithm>
#include <vob/aoe/common/physic/RigidBodyComponent.h>
#include <vob/aoe/common/physic/WorldPhysicComponent.h>

//todo
template <typename T>
bool equalWithEpsilon(T lhs, T rhs, T epsilon = std::numeric_limits<T>::epsilon())
{
	return std::abs(lhs - rhs) < epsilon;
}

namespace vob::aoe::common
{
	class SimpleControllerSystem
	{
		using Components = ecs::ComponentTypeList<
			TransformComponent
			, VelocityComponent, SimpleControllerComponent
			, RigidBodyComponent
		>;
	public:
		explicit SimpleControllerSystem(ecs::WorldDataProvider& a_wdp)
			: m_worldInput{ *a_wdp.getWorldComponent<InputComponent const>() }
			, m_worldCursor{ *a_wdp.getWorldComponent<CursorComponent>() }
			, m_worldTime{ *a_wdp.getWorldComponent<TimeComponent const>() }
			, m_worldPhysicComponent{ *a_wdp.getWorldComponent<WorldPhysicComponent>() }
			, m_entities{ a_wdp.getEntityList(*this, Components{}) }
			, m_systemSpawnManager{ a_wdp.getSpawnManager() }
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
				auto& simpleController = entity.getComponent<SimpleControllerComponent>();
				auto& rigidBody = entity.getComponent<RigidBodyComponent>();

				// Compute local required movement
				auto linearSpeed{ 4.0f };
				glm::vec3 localMove{ 0.0f };
				localMove.x -= m_worldInput.m_keyboard.m_keys[Keyboard::Key::S];
				localMove.x += m_worldInput.m_keyboard.m_keys[Keyboard::Key::F];
				localMove.z -= m_worldInput.m_keyboard.m_keys[Keyboard::Key::E];
				localMove.z += m_worldInput.m_keyboard.m_keys[Keyboard::Key::D];
				linearSpeed += m_worldInput.m_keyboard.m_keys[Keyboard::Key::A] * 42;

				if(m_worldInput.m_keyboard.m_keys[Keyboard::Key::P].m_changed
					&& m_worldInput.m_keyboard.m_keys[Keyboard::Key::P].m_pressed)
				{
					m_worldPhysicComponent.m_pause = !m_worldPhysicComponent.m_pause;
				}

				// Make local required movement global velocity
				auto t_linearVelocity = glm::vec3{
					glm::mat4_cast(glm::quat{ simpleController.m_orientation }) * glm::vec4{ localMove, 1.0f }
				};
				t_linearVelocity.y -= m_worldInput.m_keyboard.m_keys[Keyboard::Key::V];
				t_linearVelocity.y += m_worldInput.m_keyboard.m_keys[Keyboard::Key::Space];

				auto const directionLength = glm::length(t_linearVelocity);
				if (directionLength > FLT_EPSILON)
				{
					t_linearVelocity *= linearSpeed / directionLength;
				}

				rigidBody.m_rigidBody->activate(true);
				rigidBody.m_rigidBody->setLinearVelocity(toBtVector(t_linearVelocity));

				// Shoot balls
				if(m_worldInput.m_mouse.m_buttons[sf::Mouse::Left].m_pressed)
				{
					if (m_worldTime.m_frameStartTime - simpleController.m_lastBulletTime > Duration{ 0.1f })
					{
						assert(simpleController.m_bullet.isValid());
						auto t_bullet = *simpleController.m_bullet;

						// Initial velocity
						auto const t_rigidBody = t_bullet.getComponent<RigidBodyComponent>();
						glm::vec3 t_localVelocity{ 0.0f, 0.0f, -16.0f };
						t_rigidBody->m_linearVelocity = glm::vec3{
							glm::quat{ simpleController.m_orientation } * glm::vec4{ t_localVelocity, 1.0f }
						};

						// Initial position
						auto const t_transform = t_bullet.getComponent<TransformComponent>();
						t_transform->m_matrix = transform.m_matrix;
						t_transform->m_matrix = glm::translate(t_transform->m_matrix, t_rigidBody->m_linearVelocity / 10.0f);

						m_systemSpawnManager.spawn(std::move(t_bullet));
						simpleController.m_lastBulletTime = m_worldTime.m_frameStartTime;
					}
				}
				else
				{
					t = 0;
				}

				if (m_worldInput.m_mouse.m_buttons[sf::Mouse::Right])
				{
					if (m_worldInput.m_mouse.m_buttons[sf::Mouse::Right].m_changed)
					{
						m_worldCursor.m_state = common::CursorState::Disable;
					}

					// Compute new orientation

					auto& euler = simpleController.m_orientation;
					euler.y += m_worldTime.m_elapsedTime.value
						* -m_worldInput.m_mouse.m_move.x;
					euler.y = std::fmod(euler.y, 2 * glm::pi<float>());

					euler.x += m_worldTime.m_elapsedTime.value
						* -m_worldInput.m_mouse.m_move.y;
					euler.x = std::clamp(euler.x
						, -glm::pi<float>() / 2
						, glm::pi<float>() / 2);

					setRotation(transform.m_matrix, glm::quat{ euler });
				}
				else
				{
					auto& euler = simpleController.m_orientation;
					// TODO VOB TODO transform.m_rotation = glm::quat{ euler };
					if (m_worldInput.m_mouse.m_buttons[sf::Mouse::Right].m_changed)
					{
						m_worldCursor.m_state = common::CursorState::Normal;
					}
					setRotation(transform.m_matrix, glm::quat{ euler });
				}
			}
		}

	private:
		mutable int t = 0;
		InputComponent const& m_worldInput;
		CursorComponent& m_worldCursor;
		TimeComponent const& m_worldTime;
		WorldPhysicComponent& m_worldPhysicComponent;
		ecs::EntityList<
			TransformComponent
			, VelocityComponent
			, SimpleControllerComponent
			, RigidBodyComponent
		> const& m_entities;
		ecs::SystemSpawnManager& m_systemSpawnManager;
	};
}
