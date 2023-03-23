#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/components/model_component.h>
#include <vob/aoe/rendering/components/model_data_component.h>
#include <vob/aoe/rendering/context.h>
#include <vob/aoe/rendering/data/model_data_resource_manager.h>
#include <vob/aoe/rendering/world_components/mesh_render_world_component.h>

#include <vob/aoe/ecs/world_component_ref.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include <algorithm>


namespace vob::aoegl
{
	/// Initialize model_component from model_data_component
	class VOB_AOE_API model_data_resource_system
	{
	public:
		explicit model_data_resource_system(aoecs::world_data_provider& a_wdp);

		void on_spawn(aoecs::entity_list::entity_view a_entity) const;

		void on_despawn(aoecs::entity_list::entity_view a_entity) const;

		void update() const;

	private:
		aoecs::world_component_ref<model_data_resource_manager> m_modelDataResourceManager;

		aoecs::world_component_ref<mesh_render_world_component> m_meshRenderWorldComponent;
	};
}
