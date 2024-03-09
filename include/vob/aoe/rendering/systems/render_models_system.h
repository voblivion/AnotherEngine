#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/components/camera_component.h>
#include <vob/aoe/rendering/components/model_component.h>
#include <vob/aoe/rendering/world_components/director_world_component.h>
#include <vob/aoe/rendering/world_components/mesh_render_world_component.h>

#include <vob/aoe/engine/world_data_provider.h>

#include <vob/aoe/spacetime/transform.h>
#include <vob/aoe/window/window_world_component.h>
#ifndef NDEBUG
#include <vob/aoe/input/bindings.h>
#endif


namespace vob::aoegl
{
	class VOB_AOE_API render_models_system
	{
	public:
		explicit render_models_system(aoeng::world_data_provider& a_wdp);
		render_models_system(render_models_system&&) = delete;
		render_models_system(render_models_system const&) = delete;

		~render_models_system();

		decltype(auto) operator=(render_models_system&&) = delete;
		decltype(auto) operator=(render_models_system const&) = delete;


		void update() const;

	private:
		aoeng::registry_view_ref<
			aoest::position const,
			aoest::rotation const,
			model_component const
		> m_modelEntities;

		aoeng::registry_view_ref<
			aoest::position const, aoest::rotation, camera_component const> m_cameraEntities;

		aoeng::world_component_ref<aoewi::window_world_component> m_windowWorldComponent;
		aoeng::world_component_ref<director_world_component> m_directorWorldComponent;
		aoeng::world_component_ref<mesh_render_world_component> m_meshRenderWorldComponent;
#ifndef NDEBUG
		aoeng::world_component_ref<aoein::bindings> m_bindings;
		aoein::bindings::switch_id m_polygonMapping = 0;
		mutable graphic_enum m_polygonType = GL_FILL;
#endif
	};
}
