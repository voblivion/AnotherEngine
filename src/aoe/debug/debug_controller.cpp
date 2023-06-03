#include <vob/aoe/debug/debug_controller.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

#include <vob/aoe/terrain/procedural_terrain.h>
#include <vob/aoe/rendering/data/model_data.h>
#include <vob/aoe/rendering/components/model_component.h>
#include <vob/aoe/rendering/components/model_data_component.h>


namespace vob::aoedb
{
	debug_controller_system::debug_controller_system(aoeng::world_data_provider& a_wdp)
		: m_debugControllerWorldComponent{ a_wdp }
		, m_bindings{ a_wdp }
		, m_windowWorldComponent{ a_wdp }
		, m_simulationTimeWorldComponent{ a_wdp }
		, m_queryRef{ a_wdp }
		, m_debugCameraEntities{ a_wdp }
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

		auto debugCameraEntitiesView = m_debugCameraEntities.get();
		for (auto debugCameraEntity : debugCameraEntitiesView)
		{
			auto [transform, camera] = debugCameraEntitiesView.get(debugCameraEntity);

			auto moveDir = glm::vec3{ 0.0f };

			auto const longitudinalMapping = m_debugControllerWorldComponent->m_longitudinalMoveMapping;
			auto const lateralMapping = m_debugControllerWorldComponent->m_lateralMoveMapping;
			auto const verticalMapping = m_debugControllerWorldComponent->m_verticalMoveMapping;

			moveDir.x = -axes.find(lateralMapping)->get_value();
			moveDir.y = axes.find(verticalMapping)->get_value();
			moveDir.z = -axes.find(longitudinalMapping)->get_value();

			auto const dt = m_simulationTimeWorldComponent->m_elapsedTime.get_value();

			auto const move = moveDir * dt * m_debugControllerWorldComponent->m_moveSpeed;

			transform.m_matrix = glm::translate(glm::mat4{ 1.0f }, glm::quat{ transform.m_matrix } * move) * transform.m_matrix;

			if (!enableViewSwitch.is_pressed())
			{
				continue;
			}

			float yaw, pitch, roll;
			glm::extractEulerAngleYXZ(transform.m_matrix, yaw, pitch, roll);

			auto const yawMapping = m_debugControllerWorldComponent->m_yawMapping;

			auto const pitchMapping = m_debugControllerWorldComponent->m_pitchMapping;

			if (glm::epsilonNotEqual(roll, 0.0f, 4 * glm::epsilon<float>()))
			{
				if (pitch > 0.0f)
				{
					yaw += glm::pi<float>();
				}
				else
				{
					yaw -= glm::pi<float>();
				}
			}
			roll = 0.0f;
			yaw -= axes[yawMapping]->get_value() * dt;
			pitch -= axes[pitchMapping]->get_value() * dt;
			pitch = glm::clamp(pitch, -glm::half_pi<float>(), glm::half_pi<float>());
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
			auto const orientation = glm::eulerAngleYXZ(yaw, pitch, roll);

			transform.m_matrix[0] = orientation[0];
			transform.m_matrix[1] = orientation[1];
			transform.m_matrix[2] = orientation[2];
		}

		bool needTerrainUpdate = m_debugControllerWorldComponent->m_terrainEntity == entt::tombstone;
		if (switches[m_debugControllerWorldComponent->m_terrainSizeUpMapping]->was_pressed())
		{
			m_debugControllerWorldComponent->m_terrainSize *= 1.25f;
			needTerrainUpdate = true;
		}
		if (switches[m_debugControllerWorldComponent->m_terrainSizeDownMapping]->was_pressed())
		{
			m_debugControllerWorldComponent->m_terrainSize /= 1.25f;
			//std::max(8.0f, m_debugControllerWorldComponent->m_terrainSize - 8.0f);
			needTerrainUpdate = true;
		}
		if (switches[m_debugControllerWorldComponent->m_terrainCellSizeUpMapping]->was_pressed())
		{
			m_debugControllerWorldComponent->m_terrainCellSize *= 2.0f;
			// 1.0f;
			needTerrainUpdate = true;
		}
		if (switches[m_debugControllerWorldComponent->m_terrainCellSizeDownMapping]->was_pressed())
		{
			m_debugControllerWorldComponent->m_terrainCellSize /= 2.0f;
			//std::max(1.0f, m_debugControllerWorldComponent->m_terrainCellSize - 1.0f);
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

		if (needTerrainUpdate)
		{
			if (m_debugControllerWorldComponent->m_terrainEntity != entt::tombstone)
			{
				aoeng::entity const terrainEntity = m_debugControllerWorldComponent->m_terrainEntity;
				m_queryRef.add(
					[terrainEntity](aoeng::entity_registry& a_registry) {
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

			auto& terrainTransform = terrainComponents.add<aoest::transform_component>();
			terrainTransform.m_matrix = glm::translate(
				glm::mat4{ 1.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f });

			auto modelData = std::make_shared<aoegl::model_data>();
			modelData->m_texturedMeshes.emplace_back(
				aoetr::generate_procedural_terrain(
					m_debugControllerWorldComponent->m_terrainSize,
					m_debugControllerWorldComponent->m_terrainCellSize,
					layers,
					m_debugControllerWorldComponent->m_terrainUseSmoothShading),
				aoegl::material_data{});
			terrainComponents.add<aoegl::model_data_component>(std::move(modelData));
			terrainComponents.add<aoegl::model_component>();
			
			m_queryRef.add(
				[this](aoeng::entity_registry& a_registry) {
					auto const terrainEntity = a_registry.create(m_debugControllerWorldComponent->m_terrainEntity);
					
					a_registry.emplace<aoest::transform_component>(
						terrainEntity,
						*m_debugControllerWorldComponent->m_terrainComponents.find<aoest::transform_component>());

					a_registry.emplace<aoegl::model_data_component>(
						terrainEntity,
						*m_debugControllerWorldComponent->m_terrainComponents.find<aoegl::model_data_component>());

					a_registry.emplace<aoegl::model_component>(
						terrainEntity,
						*m_debugControllerWorldComponent->m_terrainComponents.find<aoegl::model_component>());

					m_debugControllerWorldComponent->m_terrainEntity = terrainEntity;
				});
		}
	}
}
