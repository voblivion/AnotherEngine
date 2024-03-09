#include <vob/aoe/debug/debug_controller.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

#include <vob/aoe/debug/controllable_tag.h>
#include <vob/aoe/terrain/procedural_terrain.h>
#include <vob/aoe/rendering/data/model_data.h>
#include <vob/aoe/rendering/components/model_component.h>
#include <vob/aoe/rendering/components/model_data_component.h>
#include <vob/aoe/physics/_collider_component.h>
#include <vob/aoe/physics/_pawn_component.h>
#include <vob/aoe/physics/components/rigidbody.h>
#include <vob/aoe/spacetime/attachment_component.h>
#include <vob/aoe/spacetime/lifetime_component.h>


#include <bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <bullet/BulletCollision/CollisionShapes/btSphereShape.h>
#include <bullet/BulletCollision/CollisionShapes/btMultiSphereShape.h>
#include <bullet/BulletCollision/CollisionShapes/btCylinderShape.h>


namespace vob::aoedb
{
	using namespace misph::literals;

	debug_controller_system::debug_controller_system(aoeng::world_data_provider& a_wdp)
		: m_debugControllerWorldComponent{ a_wdp }
		, m_bindings{ a_wdp }
		, m_windowWorldComponent{ a_wdp }
		, m_presentationTimeWorldComponent{ a_wdp }
		, m_simulationTimeWorldComponent{ a_wdp }
		, m_simulationPauseWorldComponent{ a_wdp }
		, m_debugMeshWorldComponent{ a_wdp }
		, m_queryRef{ a_wdp }
		, m_debugControllerEntities{ a_wdp }
		, m_debugCamteraEntities{ a_wdp }
	{}

	void debug_controller_system::update() const
	{
		auto const& switches = m_bindings->switches;
		auto const& axes = m_bindings->axes;

		auto const enableViewMapping = m_debugControllerWorldComponent->m_enableViewMapping;
		auto const& enableViewSwitch = *switches.find(enableViewMapping);
		if (enableViewSwitch.has_changed())
		{
			m_windowWorldComponent->m_window.get().set_cursor_state(
				enableViewSwitch.is_pressed() ? aoewi::cursor_state::disabled : aoewi::cursor_state::normal);
		}

		if (switches.find(m_debugControllerWorldComponent->m_playSim)->was_pressed())
		{
			m_simulationPauseWorldComponent->m_state = aoest::timer_state::play;
		}

		if (switches.find(m_debugControllerWorldComponent->m_stepSim)->was_pressed())
		{
			m_simulationPauseWorldComponent->m_state = aoest::timer_state::step;
		}

		auto debugControllerEntities = m_debugControllerEntities.get();
		auto debugCameraEntities = m_debugCamteraEntities.get();
		for (auto debugControllerEntity : debugControllerEntities)
		{
			auto [debugController, bodyPosition, bodyRotation] = debugControllerEntities.get(debugControllerEntity);
			auto const transform = aoest::combine(bodyPosition, bodyRotation);
			auto headEntity = debugCameraEntities.find(debugController.m_head);
			if (headEntity == debugCameraEntities.end())
			{
				ignorable_assert(false && "Debug controller without head camera attached.");
				continue;
			}
			auto [headAttachment] = debugCameraEntities.get(*headEntity);

			auto moveDir = glm::vec3{ 0.0f };

			auto const longitudinalMapping = m_debugControllerWorldComponent->m_longitudinalMoveMapping;
			auto const lateralMapping = m_debugControllerWorldComponent->m_lateralMoveMapping;
			auto const verticalMapping = m_debugControllerWorldComponent->m_verticalMoveMapping;

			moveDir.x = -axes.find(lateralMapping)->get_value();
			moveDir.y = axes.find(verticalMapping)->get_value();
			moveDir.z = -axes.find(longitudinalMapping)->get_value();

			auto const dt = m_presentationTimeWorldComponent->m_elapsedTime.get_value();

			auto const move = moveDir * dt * m_debugControllerWorldComponent->m_moveSpeed;

			bodyPosition += bodyRotation * move;

			if (switches.find(m_debugControllerWorldComponent->m_spawnItem)->was_pressed())
			{
				auto& itemComponents = m_debugControllerWorldComponent->m_itemComponents;
				itemComponents = aoecs::component_set{};

				btVector3 itemSpherePositions[2] = { btVector3(0.0f, 0.0f, 0.0f) };
				btScalar itemSphereRadiuses[2] = { btScalar(0.5f) };
				auto itemShape = std::make_shared<btMultiSphereShape>(itemSpherePositions, itemSphereRadiuses, 1);
				itemComponents.add<aoegl::model_data_component>(m_debugControllerWorldComponent->m_itemModel);

				itemComponents.add<aoest::lifetime_component>(60.0_s);

				m_queryRef.add(
					[this, bodyPosition, bodyRotation](aoeng::registry& a_registry) {
						auto const itemEntity = a_registry.create();

						// auto const offset = glm::vec3{ 0.0f, 0.0f, -1.0f };
						a_registry.emplace<aoest::position>(itemEntity, bodyPosition /* + bodyRotation * offset */);
						a_registry.emplace<aoest::rotation>(itemEntity, bodyRotation);

						a_registry.emplace<aoegl::model_data_component>(
							itemEntity,
							*m_debugControllerWorldComponent->m_itemComponents.find<aoegl::model_data_component>());

						a_registry.emplace<aoest::lifetime_component>(
							itemEntity,
							*m_debugControllerWorldComponent->m_itemComponents.find<aoest::lifetime_component>());

						a_registry.emplace<aoedb::controllable_tag>(itemEntity);

						a_registry.emplace<aoeph::rigidbody>(
							itemEntity,
							false,
							1.0f /* mass */,
							glm::rotate(glm::mat4{1.0f}, glm::half_pi<btScalar>(), glm::vec3{1.0f, 0.0f, 0.0f}) /* center of mass offset */,
							std::make_shared<btCylinderShape>(btVector3{1.0f, 1.1f, 1.0f}),
							std::make_shared<aoeph::material>(0.5f, 1.0f, 0.01f, 0.2f));

						// a_registry.emplace<aoeph::pawn_component>(itemEntity);

						/* TEST ATTACHMENT:
						auto const attachmentEntity = a_registry.create();f
						a_registry.emplace<aoest::position>(attachmentEntity);
						a_registry.emplace<aoest::rotation>(attachmentEntity);
						a_registry.emplace<aoegl::model_data_component>(
							attachmentEntity,
							*m_debugControllerWorldComponent->m_itemComponents.find<aoegl::model_data_component>());
						auto& attachment = a_registry.emplace<aoest::attachment_component>(attachmentEntity);
						attachment.m_parent = itemEntity;
						attachment.m_localTransform = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f, 0.0f, -3.0f });
						a_registry.emplace<aoest::lifetime_component>(
							attachmentEntity,
							*m_debugControllerWorldComponent->m_itemComponents.find<aoest::lifetime_component>());
						*/
					});
			}

			if (!enableViewSwitch.is_pressed())
			{
				continue;
			}

			float bodyYaw, bodyPitch, bodyRoll;
			glm::extractEulerAngleYXZ(transform, bodyYaw, bodyPitch, bodyRoll);
			float headYaw, headPitch, headRoll;
			auto& localHeadTransform = headAttachment.m_localTransform;
			glm::extractEulerAngleYXZ(localHeadTransform, headYaw, headPitch, headRoll);

			auto const yawMapping = m_debugControllerWorldComponent->m_yawMapping;

			auto const pitchMapping = m_debugControllerWorldComponent->m_pitchMapping;

			if (glm::epsilonNotEqual(headRoll, 0.0f, 4 * glm::epsilon<float>()))
			{
				if (headPitch > 0.0f)
				{
					bodyYaw += glm::pi<float>();
				}
				else
				{
					bodyYaw -= glm::pi<float>();
				}
			}
			bodyRoll = 0.0f;
			bodyYaw -= axes[yawMapping]->get_value() * dt;
			headPitch -= axes[pitchMapping]->get_value() * dt;
			static constinit auto epsilon = std::numeric_limits<float>::epsilon();
			headPitch = glm::clamp(headPitch, -glm::half_pi<float>() + epsilon, glm::half_pi<float>() - epsilon);
			localHeadTransform =
				glm::translate(glm::mat4{1.0f}, debugController.m_cameraAimAtOffset)
				* glm::eulerAngleYXZ(0.0f, headPitch, 0.0f)
				* glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f, 0.0f, debugController.m_cameraDistance });
			/*if (glm::epsilonNotEqual(roll, 0.0f, 2 * glm::epsilon<float>()))
			{
				pitch += glm::pi<float>();
				yaw += glm::pi<float>();
				roll = 0.0f;
			}
			yaw += yawChange;
			pitch += pitchChange;
			glm::clamp(pitch, -glm::half_pi<float>(), glm::half_pi<float>());

			std::cout << "yaw: " << glm::degrees(yaw) << std::endl;
			std::cout << "pitch: " << glm::degrees(pitch) << std::endl;


			// auto const orientation = glm::yawPitchRoll(yaw, pitch, 0.0f);*/
			bodyRotation = glm::quat_cast(glm::eulerAngleYXZ(bodyYaw, 0.0f, bodyRoll));
			debugController.m_headPitch = headPitch;
		}

		bool needTerrainUpdate = m_debugControllerWorldComponent->m_terrainEntity == entt::tombstone;
		if (switches[m_debugControllerWorldComponent->m_terrainSizeUpMapping]->was_pressed())
		{
			m_debugControllerWorldComponent->m_terrainSize *= 2.0f;
			needTerrainUpdate = true;
		}
		if (switches[m_debugControllerWorldComponent->m_terrainSizeDownMapping]->was_pressed())
		{
			m_debugControllerWorldComponent->m_terrainSize = std::max(4.0f,
				m_debugControllerWorldComponent->m_terrainSize / 2.0f);
			needTerrainUpdate = true;
		}
		if (switches[m_debugControllerWorldComponent->m_terrainSubdivisionCountUpMapping]->was_pressed())
		{
			m_debugControllerWorldComponent->m_terrainSubdivisionCount *= 2;
			needTerrainUpdate = true;
		}
		if (switches[m_debugControllerWorldComponent->m_terrainSubdivisionCountDownMapping]->was_pressed())
		{
			m_debugControllerWorldComponent->m_terrainSubdivisionCount = std::max(1,
				m_debugControllerWorldComponent->m_terrainSubdivisionCount / 2);
			needTerrainUpdate = true;
		}
		if (switches[m_debugControllerWorldComponent->m_terrainUseSmoothShadingMapping]->was_pressed())
		{
			m_debugControllerWorldComponent->m_terrainUseSmoothShading ^= true;
			needTerrainUpdate = true;
		}


		for (auto& layer : m_debugControllerWorldComponent->m_terrainLayers)
		{
			if (switches[layer.m_toggleMapping]->was_pressed())
			{
				layer.m_isEnabled ^= true;
				needTerrainUpdate = true;
			}
			if (switches[layer.m_frequencyUpMapping]->was_pressed())
			{
				layer.m_frequency *= 1.2f;
				needTerrainUpdate = true;
			}
			if (switches[layer.m_frequencyDownMapping]->was_pressed())
			{
				layer.m_frequency /= 1.2f;
				needTerrainUpdate = true;
			}
			if (switches[layer.m_heightUpMapping]->was_pressed())
			{
				layer.m_height *= 1.2f;
				needTerrainUpdate = true;
			}
			if (switches[layer.m_heightDownMapping]->was_pressed())
			{
				layer.m_height /= 1.2f;
				needTerrainUpdate = true;
			}
		}

		needTerrainUpdate = false; // TODO: disabling terrain for now
		if (needTerrainUpdate)
		{
			if (m_debugControllerWorldComponent->m_terrainEntity != entt::tombstone)
			{
				aoeng::entity const terrainEntity = m_debugControllerWorldComponent->m_terrainEntity;
				m_queryRef.add(
					[terrainEntity](aoeng::registry& a_registry) {
						a_registry.destroy(terrainEntity);
					});
			}

			std::vector<aoetr::layer> layers;
			for (auto const& layer : m_debugControllerWorldComponent->m_terrainLayers)
			{
				if (!layer.m_isEnabled)
				{
					continue;
				}

				layers.emplace_back(layer.m_height, layer.m_frequency, layer.m_offset);
			}

			aoecs::component_set& terrainComponents = m_debugControllerWorldComponent->m_terrainComponents;
			terrainComponents = aoecs::component_set{};

			auto const subdivisions = m_debugControllerWorldComponent->m_terrainSubdivisionCount
				* static_cast<int32_t>(m_debugControllerWorldComponent->m_terrainSize / 4);

			auto modelData = std::make_shared<aoegl::model_data>();
			modelData->m_texturedMeshes.emplace_back(
				aoetr::generate_procedural_mesh(
					glm::vec2{0.0f, 0.0f},
					glm::vec2{ m_debugControllerWorldComponent->m_terrainSize },
					glm::ivec2{ subdivisions },
					layers,
					m_debugControllerWorldComponent->m_terrainUseSmoothShading),
				aoegl::material_data{});
			terrainComponents.add<aoegl::model_data_component>(std::move(modelData));
			terrainComponents.add<aoegl::model_component>();

			m_debugControllerWorldComponent->m_nextHeights = aoetr::generate_procedural_heights(
				glm::vec2{ 0.0f, 0.0f },
				glm::vec2{ m_debugControllerWorldComponent->m_terrainSize },
				glm::ivec2{ subdivisions },
				layers);

			auto& heights = m_debugControllerWorldComponent->m_nextHeights;

			auto const minH = *std::min_element(heights.begin(), heights.end());
			auto const maxH = *std::max_element(heights.begin(), heights.end());

			auto terrainShape = std::make_shared<btHeightfieldTerrainShape>(
				subdivisions + 1,
				subdivisions + 1,
				heights.data(),
				btScalar(1.0),
				btScalar(minH),
				btScalar(maxH),
				1 /* y */,
				PHY_FLOAT,
				false /* flip quad edges */);
			terrainShape->setUseDiamondSubdivision(true);
			auto const cellSize = m_debugControllerWorldComponent->m_terrainSize / subdivisions;
			btVector3 scale{ cellSize, 1.0f, cellSize };
			terrainShape->setLocalScaling(scale);

			//terrainComponents.add<aoeph::collider_component>(
			//	0.0f /* mass */,
			//	glm::vec3{ 0.0f, (maxH + minH) / 2, 0.0f } /* offset */,
			//	glm::vec3{ 1.0f } /* linear factor */,
			//	glm::vec3{ 1.0f } /* angular factor */,
			//	std::move(terrainShape),
			//	std::make_shared<aoeph::material>());

			m_queryRef.add(
				[this, terrainShape, maxH, minH](aoeng::registry& a_registry) {
					auto const terrainEntity = a_registry.create(m_debugControllerWorldComponent->m_terrainEntity);
					
					a_registry.emplace<aoest::position>(terrainEntity, 0.0f, 0.0f, 0.0f);
					a_registry.emplace<aoest::rotation>(terrainEntity);

					a_registry.emplace<aoegl::model_data_component>(
						terrainEntity,
						*m_debugControllerWorldComponent->m_terrainComponents.find<aoegl::model_data_component>());

					a_registry.emplace<aoeph::rigidbody>(
						terrainEntity,
						false,
						0.0f /* mass */,
						aoest::combine(glm::vec3{ 0.0f, (maxH + minH) / 2, 0.0f }, {}) /* center of mass offset */,
						terrainShape,
						std::make_shared<aoeph::material>(0.2f, 0.9f, 0.01f, 0.1f));

					std::swap(
						m_debugControllerWorldComponent->m_currentHeights,
						m_debugControllerWorldComponent->m_nextHeights);

					m_debugControllerWorldComponent->m_terrainEntity = terrainEntity;
				});
		}
	}
}
