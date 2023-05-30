#pragma once

#include <vob/aoe/common/map/Hierarchycomponent.h>
#include <vob/aoe/common/space/LocalTransformcomponent.h>
#include <vob/aoe/common/_render/debugscene/DebugMesh.h>
#include <vob/aoe/common/_render/debugscene/DebugSceneRendercomponent.h>
#include <vob/aoe/common/todo/SimpleControllercomponent.h>
#include <vob/aoe/common/input/WorldInputcomponent.h>
#include <vob/aoe/common/space/Transformcomponent.h>
#include <vob/aoe/common/window/WorldCursorcomponent.h>
#include <vob/aoe/common/time/WorldTimecomponent.h>
#include <vob/aoe/common/physic/RigidBodycomponent.h>
#include <vob/aoe/common/physic/WorldPhysiccomponent.h>
#include <vob/aoe/common/input/Keyboard.h>
#include <vob/aoe/common/input/Mouse.h>

#include <vob/aoe/input/mapped_inputs_world_component.h>
#include <vob/aoe/input/physical_axis_mapping.h>
#include <vob/aoe/input/physical_switch_mapping.h>
#include <vob/aoe/input/axis_switch_mapping.h>
#include <vob/aoe/actor/action_component.h>
#include <vob/aoe/actor/actor_component.h>

#include <vob/aoe/ecs/world_data_provider.h>
#include <vob/aoe/ecs/entity_manager.h>
#include <vob/aoe/ecs/entity_map_observer_list_ref.h>

#include <vob/misc/std/polymorphic_ptr_util.h>

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
	public:
		explicit SimpleControllerSystem(aoecs::world_data_provider& a_wdp)
			: m_mappedInputsComponent{ a_wdp.get_world_component<aoein::mapped_inputs_world_component const>() }
			, m_worldCursor{ a_wdp.get_world_component<WorldCursorComponent>() }
			, m_worldTimeComponent{ a_wdp.get_world_component<WorldTimeComponent const>() }
			, m_worldPhysicComponent{ a_wdp.get_world_component<WorldPhysicComponent>() }
			, m_entities{ a_wdp }
			, m_heads{ a_wdp }
			, m_debugSceneRenderComponent{ a_wdp.get_world_component<DebugSceneRenderComponent>() }
			, m_actions{ a_wdp }
			, m_spawnManager{ a_wdp.get_spawner() }
		{
			auto& mapping = a_wdp.get_world_component<aoein::mapped_inputs_world_component>();

			auto allocator = std::allocator<aoein::basic_axis_mapping>();

			m_lateralMoveMapping = mapping.m_axes.size();
			mapping.m_axes.emplace_back(mistd::polymorphic_ptr_util::allocate<aoein::physical_axis_mapping>(
				allocator, aoein::physical_axis_reference{ 0, aoein::gamepad::axis::LX }, 0.001f));

			m_longitudinalMoveMapping = mapping.m_axes.size();
			mapping.m_axes.emplace_back(mistd::polymorphic_ptr_util::allocate<aoein::physical_axis_mapping>(
				allocator, aoein::physical_axis_reference{ 0, aoein::gamepad::axis::LY }, 0.001f));

			m_lateralViewMapping = mapping.m_axes.size();
			mapping.m_axes.emplace_back(mistd::polymorphic_ptr_util::allocate<aoein::physical_axis_mapping>(
				allocator, aoein::physical_axis_reference{ 0, aoein::gamepad::axis::RX }, 0.001f));

			m_verticalViewMapping = mapping.m_axes.size();
			mapping.m_axes.emplace_back(mistd::polymorphic_ptr_util::allocate<aoein::physical_axis_mapping>(
				allocator, aoein::physical_axis_reference{ 0, aoein::gamepad::axis::RY }, 0.001f));

			m_pauseMapping = mapping.m_switches.size();
			mapping.m_switches.emplace_back(mistd::polymorphic_ptr_util::allocate<aoein::physical_switch_mapping>(
				allocator, aoein::physical_switch_reference{ aoein::keyboard::key::P }));

			m_debugDisplayMapping = mapping.m_switches.size();
			mapping.m_switches.emplace_back(mistd::polymorphic_ptr_util::allocate<aoein::physical_switch_mapping>(
				allocator, aoein::physical_switch_reference{ aoein::keyboard::key::D }));

			m_jumpMapping = mapping.m_switches.size();
			mapping.m_switches.emplace_back(mistd::polymorphic_ptr_util::allocate<aoein::physical_switch_mapping>(
				allocator, aoein::physical_switch_reference{ 0, aoein::gamepad::button::A }));

			m_shootMapping = mapping.m_switches.size();
			mapping.m_switches.emplace_back(mistd::polymorphic_ptr_util::allocate<aoein::axis_switch_mapping>(
				allocator,
				mistd::polymorphic_ptr_util::allocate<aoein::physical_axis_mapping>(
					allocator, aoein::physical_axis_reference{ 0, aoein::gamepad::axis::RT }, 0.000f),
				0.9f,
				0.5f,
				true));

			m_interactMapping = mapping.m_switches.size();
			mapping.m_switches.emplace_back(mistd::polymorphic_ptr_util::allocate<aoein::physical_switch_mapping>(
				allocator, aoein::physical_switch_reference{ 0, aoein::gamepad::button::X }));
		}


		static glm::vec3 toVec3(glm::vec4 const v)
		{
			return glm::vec3{ v.x, v.y, v.z };
		}

		void update() const
		{
			for (auto const& entity : m_entities)
			{
				auto& transform = entity.get<TransformComponent>();


				auto& simpleController = entity.get<SimpleControllerComponent>();
				auto& rigidBody = entity.get<RigidBodyComponent>();

				auto const& interactor = entity.get<aoeac::actor_component>();

				m_debugSceneRenderComponent.m_debugMesh.addLine(
					DebugVertex{ transform.m_matrix[3], glm::vec3{1.0f, 0.0f, 0.0f} }
					, DebugVertex{ glm::vec3{ transform.m_matrix[3] } + glm::vec3{1.0f, 0.0f, 0.0f}, glm::vec3{1.0f, 0.0f, 0.0f} }
				);

				m_debugSceneRenderComponent.m_debugMesh.addLine(
					DebugVertex{ transform.m_matrix[3], glm::vec3{0.0f, 1.0f, 0.0f} }
					, DebugVertex{ glm::vec3{ transform.m_matrix[3] } + glm::vec3{0.0f, 1.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f} }
				);

				m_debugSceneRenderComponent.m_debugMesh.addLine(
					DebugVertex{ transform.m_matrix[3], glm::vec3{0.0f, 0.0f, 1.0f} }
					, DebugVertex{ glm::vec3{ transform.m_matrix[3] } + glm::vec3{0.0f, 0.0f, 1.0f}, glm::vec3{0.0f, 0.0f, 1.0f} }
				);

				// Compute local required movement
				auto linearSpeed{ 8.0f };
				glm::vec3 localMove{ 0.0f };
				localMove.x = m_mappedInputsComponent.m_axes[m_lateralMoveMapping]->get_value();
				localMove.z = m_mappedInputsComponent.m_axes[m_longitudinalMoveMapping]->get_value();

				if (m_mappedInputsComponent.m_switches[m_pauseMapping]->changed()
					&& m_mappedInputsComponent.m_switches[m_pauseMapping]->is_pressed())
				{
					m_worldPhysicComponent.m_pause = !m_worldPhysicComponent.m_pause;
				}
				if (m_mappedInputsComponent.m_switches[m_debugDisplayMapping]->changed()
					&& m_mappedInputsComponent.m_switches[m_debugDisplayMapping]->is_pressed())
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
				
				misph::measure_acceleration const gravity{ -9.81f * 2.0f };
				misph::measure_velocity const addedVerticalSpeed = gravity * m_worldTimeComponent.m_elapsedTime;

				t_linearVelocity.y = simpleController.m_fallVelocity;
				t_linearVelocity.y += addedVerticalSpeed.get_value();
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
						DebugVertex{ toGlmVec3(from), glm::vec3{1.0f, 0.0f, 1.0f} }
						, DebugVertex{ toGlmVec3(to), glm::vec3{1.0f, 0.0f, 1.0f} }
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

				if (canJump && m_mappedInputsComponent.m_switches[m_jumpMapping]->changed()
					&& m_mappedInputsComponent.m_switches[m_jumpMapping]->is_pressed())
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
				if(m_mappedInputsComponent.m_switches[m_shootMapping]->is_pressed())
				{
					if (m_worldTimeComponent.m_frameStartTime - simpleController.m_lastBulletTime > Duration{ 0.1f })
					{
						assert(simpleController.m_bullet != nullptr);
						auto& t_bullet = *simpleController.m_bullet;

						glm::vec3 t_localVelocity{ 0.0f, 0.0f, -8.0f };
						auto const nextBulletVelocity = glm::vec3{
							glm::quat{ simpleController.m_orientation }
							*glm::quat{ simpleController.m_headOrientation}
							*glm::vec4{ t_localVelocity, 1.0f }
						};
						auto const nextBulletMatrix = glm::translate(
							transform.m_matrix, glm::vec3{ 0.0f, 1.5f, 0.0f } + m_nextBulletVelocity / 10.0f);

						m_bulletSpawnCallback = [nextBulletVelocity, nextBulletMatrix](aoecs::entity_list::entity_view& a_bullet)
						{
							// Initial velocity
							auto const t_rigidBody = a_bullet.get_component<RigidBodyComponent>();
							t_rigidBody->m_linearVelocity = nextBulletVelocity;

							// Initial position
							auto const t_transform = a_bullet.get_component<TransformComponent>();
							t_transform->m_matrix = nextBulletMatrix;
						};

						m_spawnManager.spawn(std::move(t_bullet), &m_bulletSpawnCallback);
						simpleController.m_lastBulletTime = m_worldTimeComponent.m_frameStartTime;
					}
				}
				else
				{
					t = 0;
				}

				auto const& interact = m_mappedInputsComponent.m_switches[m_interactMapping];
				if (interact->changed() && !interactor.m_actions.empty())
				{
					auto const actionEntity = m_actions.find(interactor.m_actions[0]);
					if (actionEntity != m_actions.end())
					{
						auto& action = actionEntity->get<aoeac::action_component>();
						if (interact->is_pressed())
						{
							action.m_isInteracting = true;
						}
						else
						{
							action.m_isInteracting = false;
						}
					}
				}

				if (true)
				//if (m_worldInput.m_mouse.m_buttons[Mouse::Button::Right])
				{
					/*if (m_worldInput.m_mouse.m_buttons[Mouse::Button::Right].m_changed)
					{
						m_worldCursor.m_state = common::CursorState::Disable;
					}*/

					// Compute new orientation

					auto& euler = simpleController.m_orientation;
					euler.y += m_worldTimeComponent.m_elapsedTime.get_value()
						* -m_mappedInputsComponent.m_axes[m_lateralViewMapping]->get_value() * 2;
					euler.y = std::fmod(euler.y, 2 * glm::pi<float>());

					auto& headEuler = simpleController.m_headOrientation;
					headEuler.x += m_worldTimeComponent.m_elapsedTime.get_value()
						* -m_mappedInputsComponent.m_axes[m_verticalViewMapping]->get_value() * 2;

					auto& hierarchy = entity.get<HierarchyComponent>();
					if (!hierarchy.m_children.empty())
					{
						auto head = m_heads.find(hierarchy.m_children[0]);
						if (head != m_heads.end())
						{
							auto& localTransform = head->get<LocalTransformComponent>();
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
					/*auto& euler = simpleController.m_orientation;
					// TODO VOB TODO transform.m_rotation = glm::quat{ euler };
					if (m_worldInput.m_mouse.m_buttons[Mouse::Button::Right].m_changed)
					{
						m_worldCursor.m_state = common::CursorState::Normal;
					}
					setRotation(transform.m_matrix, glm::quat{ euler });*/
				}
			}
		}

	private:
		mutable aoecs::entity_map::create_callback m_bulletSpawnCallback;

		mutable glm::vec3 m_nextBulletVelocity;
		mutable glm::mat4 m_nextBulletMatrix;

		std::size_t m_lateralMoveMapping;
		std::size_t m_longitudinalMoveMapping;
		std::size_t m_lateralViewMapping;
		std::size_t m_verticalViewMapping;

		std::size_t m_pauseMapping;
		std::size_t m_debugDisplayMapping;
		std::size_t m_jumpMapping;
		std::size_t m_shootMapping;
		std::size_t m_interactMapping;

		mutable int t = 0;
		aoein::mapped_inputs_world_component const& m_mappedInputsComponent;
		WorldCursorComponent& m_worldCursor;
		WorldTimeComponent const& m_worldTimeComponent;
		WorldPhysicComponent& m_worldPhysicComponent;
		// TMP
		DebugSceneRenderComponent& m_debugSceneRenderComponent;
		aoecs::entity_map_observer_list_ref <
			TransformComponent
			, SimpleControllerComponent
			, RigidBodyComponent
			, HierarchyComponent const
			, aoeac::actor_component const
		> m_entities;
		aoecs::entity_map_observer_list_ref<HierarchyComponent const, LocalTransformComponent> m_heads;
		aoecs::entity_map_observer_list_ref<aoeac::action_component> m_actions;
		aoecs::entity_manager::spawner& m_spawnManager;
	};
}
