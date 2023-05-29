#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/components/camera_component.h>
#include <vob/aoe/rendering/components/model_component.h>
#include <vob/aoe/rendering/world_components/director_world_component.h>
#include <vob/aoe/rendering/world_components/mesh_render_world_component.h>

#include <vob/aoe/ecs/entity_map_observer_list_ref.h>
#include <vob/aoe/ecs/world_component_ref.h>
#include <vob/aoe/ecs/world_data_provider.h>
#include <vob/aoe/spacetime/transform_component.h>
#include <vob/aoe/window/window_world_component.h>
#ifndef NDEBUG
#include <vob/aoe/input/mapped_inputs_world_component.h>
#endif


namespace vob::aoegl
{
	class VOB_AOE_API render_models_system
	{
	public:
		explicit render_models_system(aoecs::world_data_provider& a_wdp);
		render_models_system(render_models_system&&) = delete;
		render_models_system(render_models_system const&) = delete;

		~render_models_system();

		decltype(auto) operator=(render_models_system&&) = delete;
		decltype(auto) operator=(render_models_system const&) = delete;
		

		void update() const;

	private:
		aoecs::entity_map_observer_list_ref<
			model_component const, aoest::transform_component const> m_modelEntities;

		aoecs::entity_map_observer_list_ref<
			aoest::transform_component const, camera_component const> m_cameraEntities;

		aoecs::world_component_ref<aoewi::window_world_component> m_windowWorldComponent;
		aoecs::world_component_ref<director_world_component> m_directorWorldComponent;
		aoecs::world_component_ref<mesh_render_world_component> m_meshRenderWorldComponent;
#ifndef NDEBUG
		aoecs::world_component_ref<aoein::mapped_inputs_world_component> m_mappedInputsWorldComponent;
		std::size_t m_polygonMapping = 0;
		mutable graphic_enum m_polygonType = GL_FILL;
#endif
	};
}