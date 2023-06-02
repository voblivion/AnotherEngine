#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/components/model_component.h>
#include <vob/aoe/rendering/components/model_data_component.h>
#include <vob/aoe/rendering/context.h>
#include <vob/aoe/rendering/data/model_data_resource_manager.h>
#include <vob/aoe/rendering/world_components/mesh_render_world_component.h>

#include <vob/aoe/ecs/world_component_ref.h>

#include <vob/aoe/engine/world_data_provider.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include <algorithm>


namespace vob::aoegl
{
	class VOB_AOE_API model_data_resource_system
	{
	public:
		explicit model_data_resource_system(aoeng::world_data_provider& a_wdp);

		void update() const;

		void on_construct(aoeng::entity_registry& a_registry, aoeng::entity a_entity);

		void on_destroy(aoeng::entity_registry& a_registry, aoeng::entity a_entity);

	private:
		aoeng::world_component_ref<model_data_resource_manager> m_modelDataResourceManager;
		aoeng::world_component_ref<mesh_render_world_component> m_meshRenderWorldComponent;
	};
}
