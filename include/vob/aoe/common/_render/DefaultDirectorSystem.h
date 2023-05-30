#pragma once
#include <vob/aoe/ecs/world_data_provider.h>
#include <vob/aoe/ecs/entity_map_observer_list_ref.h>

#include <vob/aoe/common/_render/Cameracomponent.h>
#include <vob/aoe/common/_render/Directorcomponent.h>
#include <vob/aoe/common/space/Transformcomponent.h>

namespace vob::aoe::common
{
	struct DefaultDirectorSystem
	{
		// Constructor
		explicit DefaultDirectorSystem(aoecs::world_data_provider& a_wdp)
			: m_directorComponent{ a_wdp.get_world_component<DirectorComponent>() }
			, m_cameraList{ a_wdp }
		{}

		void update() const
		{
			auto const t_camera = m_cameraList.find(m_directorComponent.m_currentCamera);

			if (t_camera == m_cameraList.end() && !m_cameraList.empty())
			{
				m_directorComponent.m_currentCamera = m_cameraList.begin()->get_id();
			}
		}

		// Attributes
		DirectorComponent& m_directorComponent;
		aoecs::entity_map_observer_list_ref<
			TransformComponent const
			, CameraComponent const
		> m_cameraList;
	};
}
