#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/ecs/entity_map_observer_list_ref.h>
#include <vob/aoe/ecs/world_component_ref.h>
#include <vob/aoe/ecs/world_data_provider.h>

#include <vob/aoe/input/mapped_inputs_world_component.h>

#include <vob/aoe/rendering/components/camera_component.h>

#include <vob/aoe/spacetime/transform_component.h>

#include <vob/aoe/window/window_world_component.h>

#include <vob/misc/visitor/macros.h>


namespace vob::aoedb
{
	struct debug_controller_component
	{

	};

	struct debug_controller_world_component
	{
		// Camera
		std::size_t m_enableViewMapping = 0;
		std::size_t m_lateralMoveMapping = 0;
		std::size_t m_longitudinalMoveMapping = 0;
		std::size_t m_verticalMoveMapping = 0;
		std::size_t m_pitchMapping = 0;
		std::size_t m_yawMapping = 0;

		// Terrain
		aoecs::entity_id m_terrainEntityId = aoecs::k_invalidEntityId;
		aoecs::component_set m_terrainComponents;
		float m_terrainSize = 64.0f;
		std::size_t m_terrainSizeUpMapping = 0;
		std::size_t m_terrainSizeDownMapping = 0;
		float m_terrainCellSize = 1.0f;
		std::size_t m_terrainCellSizeUpMapping = 0;
		std::size_t m_terrainCellSizeDownMapping = 0;
		bool m_terrainUseSmoothShading = false;
		std::size_t m_terrainUseSmoothShadingMapping = 0;
		struct terrain_layer
		{
			bool m_isEnabled = false;
			std::size_t m_toggleMapping = 0;
			float m_frequency;
			std::size_t m_frequencyUpMapping = 0;
			std::size_t m_frequencyDownMapping = 0;
			float m_height;
			std::size_t m_heightUpMapping = 0;
			std::size_t m_heightDownMapping = 0;
			glm::vec2 m_offset;
		};
		std::vector<terrain_layer> m_terrainLayers;
	};

	class VOB_AOE_API debug_controller_system
	{
	public:
		explicit debug_controller_system(aoecs::world_data_provider& a_wdp);

		void update() const;

	private:
		aoecs::world_component_ref<debug_controller_world_component>
			m_debugControllerWorldComponent;
		aoecs::world_component_ref<aoein::mapped_inputs_world_component const>
			m_mappedInputsWorldComponent;
		aoecs::world_component_ref<aoewi::window_world_component> m_windowWorldComponent;

		aoecs::entity_manager::spawner& m_spawner;
		aoecs::entity_manager::despawner& m_despawner;

		aoecs::entity_map_observer_list_ref<
			debug_controller_component const,
			aoest::transform_component,
			aoegl::camera_component> m_debugCameraEntities;
	};
}

namespace vob::misvi
{
	VOB_MISVI_ACCEPT(vob::aoedb::debug_controller_component)
	{
		return true;
	}
}
