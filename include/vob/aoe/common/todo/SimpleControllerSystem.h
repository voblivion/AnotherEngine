#pragma once

#include <vob/aoe/common/map/HierarchyComponent.h>
#include <vob/aoe/common/space/LocalTransformComponent.h>
#include <vob/aoe/common/render/debugscene/DebugMesh.h>
#include <vob/aoe/common/render/debugscene/DebugSceneRenderComponent.h>
#include <vob/aoe/common/space/VelocityComponent.h>
#include <vob/aoe/common/todo/SimpleControllerComponent.h>
#include <vob/aoe/common/input/WorldInputComponent.h>
#include <vob/aoe/common/space/TransformComponent.h>
#include <vob/aoe/common/window/WorldCursorComponent.h>
#include <vob/aoe/common/time/WorldTimeComponent.h>
#include <vob/aoe/common/physic/RigidBodyComponent.h>
#include <vob/aoe/common/physic/WorldPhysicComponent.h>
#include <vob/aoe/common/input/physical/Keyboard.h>
#include <vob/aoe/common/input/physical/Mouse.h>

#include <vob/aoe/core/ecs/WorldDataProvider.h>

#include <glm/gtc/quaternion.hpp>

#include <cmath>
#include <algorithm>

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
			, VelocityComponent
			, SimpleControllerComponent
			, RigidBodyComponent
			, HierarchyComponent const
		>;
	public:
		explicit SimpleControllerSystem(ecs::WorldDataProvider& a_wdp)
			: m_worldInput{ *a_wdp.getWorldComponent<WorldInputComponent const>() }
			, m_worldCursor{ *a_wdp.getWorldComponent<WorldCursorComponent>() }
			, m_worldTimeComponent{ *a_wdp.getWorldComponent<WorldTimeComponent const>() }
			, m_worldPhysicComponent{ *a_wdp.getWorldComponent<WorldPhysicComponent>() }
			, m_entities{ a_wdp.getEntityViewList(*this, Components{}) }
			, m_heads{ a_wdp.getEntityViewList<SimpleControllerSystem, HierarchyComponent const, LocalTransformComponent>(*this) }
			, m_debugSceneRenderComponent{ *a_wdp.getWorldComponent<DebugSceneRenderComponent>() }
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

				m_debugSceneRenderComponent.m_debugMesh.addLine(
					DebugVertex{ transform.m_matrix[3], vec3{1.0f, 0.0f, 0.0f} }
					, DebugVertex{ vec3{ transform.m_matrix[3] } + vec3{1.0f, 0.0f, 0.0f}, vec3{1.0f, 0.0f, 0.0f} }
				);

				m_debugSceneRenderComponent.m_debugMesh.addLine(
					DebugVertex{ transform.m_matrix[3], vec3{0.0f, 1.0f, 0.0f} }
					, DebugVertex{ vec3{ transform.m_matrix[3] } + vec3{0.0f, 1.0f, 0.0f}, vec3{0.0f, 1.0f, 0.0f} }
				);

				m_debugSceneRenderComponent.m_debugMesh.addLine(
					DebugVertex{ transform.m_matrix[3], vec3{0.0f, 0.0f, 1.0f} }
					, DebugVertex{ vec3{ transform.m_matrix[3] } + vec3{0.0f, 0.0f, 1.0f}, vec3{0.0f, 0.0f, 1.0f} }
				);

				// Compute local required movement
				auto linearSpeed{ 8.0f };
				glm::vec3 localMove{ 0.0f };
				localMove.x -= m_worldInput.m_keyboard.m_keys[Keyboard::Key::S];
				localMove.x += m_worldInput.m_keyboard.m_keys[Keyboard::Key::F];
				localMove.x = m_worldInput.m_gamepads[0].m_axes[Gamepad::Axis::LX];
				if (std::abs(localMove.x) < 0.001f)
				{
					localMove.x = 0.0f;
				}
				localMove.z -= m_worldInput.m_keyboard.m_keys[Keyboard::Key::E];
				localMove.z += m_worldInput.m_keyboard.m_keys[Keyboard::Key::D];
				localMove.z = m_worldInput.m_gamepads[0].m_axes[Gamepad::Axis::LY];
				if (std::abs(localMove.z) < 0.001f)
				{
					localMove.z = 0.0f;
				}

				if(m_worldInput.m_keyboard.m_keys[Keyboard::Key::P].m_changed
					&& m_worldInput.m_keyboard.m_keys[Keyboard::Key::P].m_isActive)
				{
					m_worldPhysicComponent.m_pause = !m_worldPhysicComponent.m_pause;
				}
				if (m_worldInput.m_keyboard.m_keys[Keyboard::Key::D].m_changed
					&& m_worldInput.m_keyboard.m_keys[Keyboard::Key::D].m_isActive)
				{
					m_worldPhysicComponent.m_displayDebug ^= true;
				}

				// Make local required movement global velocity
				// TODO : don't count X rotation, just Z rotation matters
				auto rot = glm::vec3{ 0.0f, simpleController.m_orientation.y, 0.0f };
				auto t_linearVelocity = glm::vec3{ glm::mat4_cast(glm::quat{ rot }) * glm::vec4{ localMove, 1.0f } };
				t_linearVelocity.y = 0.0f;
				auto const directionLength = glm::length(t_linearVelocity);
				if (directionLength > FLT_EPSILON)
				{
					t_linearVelocity *= linearSpeed;// / directionLength;
				}
				
				sta::acceleration_measure<float> const gravity{ -9.81f * 2.0f };
				sta::speed_measure<float> const addedVerticalSpeed = gravity * m_worldTimeComponent.m_elapsedTime;

				t_linearVelocity.y = simpleController.m_fallVelocity;
				t_linearVelocity.y += addedVerticalSpeed.value;
				t_linearVelocity.y = std::max(-10.0f, t_linearVelocity.y); // human's terminal velocity = 55m/s

				bool canJump = false;
				{
					constexpr float c_canJumpDistance = 0.2f;
					constexpr float c_groundedDistance = 0.02f;
					auto from = toBtVector(glm::vec3{ transform.m_matrix[3] });
					auto to = from;
					to[1] -= c_canJumpDistance;
					btCollisionWorld::AllHitsRayResultCallback res(from, to);

					auto& t_dynamicsWorld =
						m_worldPhysicComponent.m_dynamicsWorldHolder->getDynamicsWorld();
					t_dynamicsWorld.rayTest(from, to, res);
					m_debugSceneRenderComponent.m_debugMesh.addLine(
						DebugVertex{ toGlmVec3(from), vec3{1.0f, 0.0f, 1.0f} }
						, DebugVertex{ toGlmVec3(to), vec3{1.0f, 0.0f, 1.0f} }
					);
					if (res.hasHit())
					{
						for (auto k = 0; k < res.m_collisionObjects.size(); ++k)
						{
							auto const* colObj = res.m_collisionObjects[k];
							if (colObj != static_cast<btCollisionObject const*>(rigidBody.m_rigidBody.get()))
							{
								auto hitPoint = res.m_hitPointWorld[k];
								if (m_worldTimeComponent.m_frameStartTime - simpleController.m_lastJumpTime > Duration{ 0.25f })
								{
									canJump = true;
									if (from[1] - hitPoint[1] < c_groundedDistance)
									{
										t_linearVelocity.y = 0.0f;
									}
								}
							}
						}
					}
				}

				auto jumpKey = m_worldInput.m_gamepads[0].m_buttons[Gamepad::Button::A];
				if (canJump && jumpKey.m_changed && jumpKey.m_isActive)
				{
					t_linearVelocity.y = 10.0f;
					simpleController.m_lastJumpTime = m_worldTimeComponent.m_frameStartTime;
				}
				else
				{
				}

				simpleController.m_fallVelocity = t_linearVelocity.y;
				// t_linearVelocity.y += m_worldInput.m_keyboard.m_keys[Keyboard::Key::Space];

				if (glm::length(t_linearVelocity) > 0.0f)
				{
					rigidBody.m_rigidBody->activate(true);
					rigidBody.m_rigidBody->setGravity(btVector3{ 0.0f, 0.0f, 0.0f });
					rigidBody.m_rigidBody->setLinearVelocity(toBtVector(t_linearVelocity));
				}
				else
				{
					rigidBody.m_rigidBody->activate(false);
					rigidBody.m_rigidBody->setGravity(btVector3{ 0.0f, 0.0f, 0.0f });
					rigidBody.m_rigidBody->setLinearVelocity(toBtVector(t_linearVelocity));
				}

				// Shoot balls
				if(m_worldInput.m_gamepads[0].m_axes[Gamepad::Axis::RT] > 0.7f)
				{
					if (m_worldTimeComponent.m_frameStartTime - simpleController.m_lastBulletTime > Duration{ 0.1f })
					{
						assert(simpleController.m_bullet != nullptr);
						auto t_bullet = *simpleController.m_bullet;

						// Initial velocity
						auto const t_rigidBody = t_bullet.getComponent<RigidBodyComponent>();
						glm::vec3 t_localVelocity{ 0.0f, 0.0f, -8.0f };
						t_rigidBody->m_linearVelocity = glm::vec3{
							glm::quat{ simpleController.m_orientation } * glm::vec4{ t_localVelocity, 1.0f }
						};

						// Initial position
						auto const t_transform = t_bullet.getComponent<TransformComponent>();
						t_transform->m_matrix = transform.m_matrix;
						t_transform->m_matrix = glm::translate(t_transform->m_matrix, t_rigidBody->m_linearVelocity / 10.0f);

						m_systemSpawnManager.spawn(std::move(t_bullet));
						simpleController.m_lastBulletTime = m_worldTimeComponent.m_frameStartTime;
					}
				}
				else
				{
					t = 0;
				}

				if (true)
				//if (m_worldInput.m_mouse.m_buttons[Mouse::Button::Right])
				{
					if (m_worldInput.m_mouse.m_buttons[Mouse::Button::Right].m_changed)
					{
						//m_worldCursor.m_state = common::CursorState::Disable;
					}

					// Compute new orientation

					auto& euler = simpleController.m_orientation;
					euler.y += m_worldTimeComponent.m_elapsedTime.value
						* -m_worldInput.m_gamepads[0].m_axes[Gamepad::Axis::RX] * 2;
					//	* -m_worldInput.m_mouse.m_move.x;
					euler.y = std::fmod(euler.y, 2 * glm::pi<float>());

					auto& headEuler = simpleController.m_headOrientation;
					headEuler.x += m_worldTimeComponent.m_elapsedTime.value
						* -m_worldInput.m_gamepads[0].m_axes[Gamepad::Axis::RY] * 2;

					auto& hierarchy = entity.getComponent<HierarchyComponent>();
					if (!hierarchy.m_children.empty())
					{
						auto head = m_heads.find(hierarchy.m_children[0]);
						if (head != nullptr)
						{
							auto& localTransform = head->getComponent<LocalTransformComponent>();
							setRotation(localTransform.m_matrix, glm::quat{ headEuler });
						}
					}

					/*euler.x += m_worldTimeComponent.m_elapsedTime.value
						* -m_worldInput.m_gamepads[0].m_axes[Gamepad::Axis::RY] * 2;
					//	* -m_worldInput.m_mouse.m_move.y;
					euler.x = std::clamp(euler.x
						, -glm::pi<float>() / 2
						, glm::pi<float>() / 2);*/

					setRotation(transform.m_matrix, glm::quat{ euler });
				}
				else
				{
					auto& euler = simpleController.m_orientation;
					// TODO VOB TODO transform.m_rotation = glm::quat{ euler };
					if (m_worldInput.m_mouse.m_buttons[Mouse::Button::Right].m_changed)
					{
						m_worldCursor.m_state = common::CursorState::Normal;
					}
					setRotation(transform.m_matrix, glm::quat{ euler });
				}
			}
		}

	private:
		mutable int t = 0;
		WorldInputComponent const& m_worldInput;
		WorldCursorComponent& m_worldCursor;
		WorldTimeComponent const& m_worldTimeComponent;
		WorldPhysicComponent& m_worldPhysicComponent;
		// TMP
		DebugSceneRenderComponent& m_debugSceneRenderComponent;
		ecs::EntityViewList<
			TransformComponent
			, VelocityComponent
			, SimpleControllerComponent
			, RigidBodyComponent
			, HierarchyComponent const
		> const& m_entities;
		ecs::EntityViewList<HierarchyComponent const, LocalTransformComponent> const& m_heads;
		ecs::SystemSpawnManager& m_systemSpawnManager;
	};
}
