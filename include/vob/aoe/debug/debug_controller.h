#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/ecs/entity_map_observer_list_ref.h>
#include <vob/aoe/ecs/world_component_ref.h>
#include <vob/aoe/ecs/world_data_provider.h>

#include <vob/aoe/engine/world_data_provider.h>

#include <vob/aoe/input/bindings.h>

#include <vob/aoe/rendering/data/model_data.h>
#include <vob/aoe/rendering/components/camera_component.h>

#include <vob/aoe/spacetime/transform_component.h>
#include <vob/aoe/spacetime/time_world_component.h>

#include <vob/aoe/window/window_world_component.h>

#include <vob/misc/visitor/macros.h>
#include <vob/misc/std/vector2d.h>


namespace vob::aoedb
{
	struct debug_controller_component
	{

	};

	struct debug_controller_world_component
	{
		// Camera
		aoein::bindings::switch_id m_enableViewMapping = 0;
		aoein::bindings::switch_id m_spawnItem = 0;
		aoein::bindings::axis_id m_lateralMoveMapping = 0;
		aoein::bindings::axis_id m_longitudinalMoveMapping = 0;
		aoein::bindings::axis_id m_verticalMoveMapping = 0;
		aoein::bindings::axis_id m_pitchMapping = 0;
		aoein::bindings::axis_id m_yawMapping = 0;
		float m_moveSpeed = 10.0f;

		// Item
		aoecs::component_set m_itemComponents;
		std::shared_ptr<aoegl::model_data const> m_itemModel;

		// Terrain
		aoeng::entity m_terrainEntity = entt::tombstone;
		aoecs::entity_id m_terrainEntityId = aoecs::k_invalidEntityId;
		aoecs::component_set m_terrainComponents;
		mistd::vector2d<float> m_currentHeights;
		mistd::vector2d<float> m_nextHeights;
		float m_terrainSize = 64.0f;
		aoein::bindings::switch_id m_terrainSizeUpMapping = 0;
		aoein::bindings::switch_id m_terrainSizeDownMapping = 0;
		int32_t m_terrainSubdivisionCount = 16;
		aoein::bindings::switch_id m_terrainSubdivisionCountUpMapping = 0;
		aoein::bindings::switch_id m_terrainSubdivisionCountDownMapping = 0;
		bool m_terrainUseSmoothShading = false;
		aoein::bindings::switch_id m_terrainUseSmoothShadingMapping = 0;
		struct terrain_layer
		{
			bool m_isEnabled = false;
			aoein::bindings::switch_id m_toggleMapping = 0;
			float m_frequency;
			aoein::bindings::switch_id m_frequencyUpMapping = 0;
			aoein::bindings::switch_id m_frequencyDownMapping = 0;
			float m_height;
			aoein::bindings::switch_id m_heightUpMapping = 0;
			aoein::bindings::switch_id m_heightDownMapping = 0;
			glm::vec2 m_offset;
		};
		std::vector<terrain_layer> m_terrainLayers;
	};

	class VOB_AOE_API debug_controller_system
	{
	public:
		explicit debug_controller_system(aoeng::world_data_provider& a_wdp);

		void update() const;

	private:
		aoeng::world_component_ref<debug_controller_world_component> m_debugControllerWorldComponent;
		aoeng::world_component_ref<aoein::bindings const> m_bindings;
		aoeng::world_component_ref<aoewi::window_world_component> m_windowWorldComponent;
		aoeng::world_component_ref<aoest::simulation_time_world_component> m_simulationTimeWorldComponent;

		aoeng::pending_entity_registry_query_queue_ref m_queryRef;

		aoeng::registry_view_ref<
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
