#pragma once
#include <vob/aoe/ecs/world_data_provider.h>

#include <vob/aoe/common/_render/Cameracomponent.h>
#include <vob/aoe/common/_render/Directorcomponent.h>
#include <vob/aoe/common/space/Transformcomponent.h>

namespace vob::aoe::common
{
	struct DefaultDirectorSystem
	{
		using CameraComponents = aoecs::ComponentTypeList<
			TransformComponent const
			, CameraComponent const
		>;

		// Constructor
		explicit DefaultDirectorSystem(aoecs::world_data_provider& a_wdp)
			: m_directorComponent{ a_wdp.get_world_component<DirectorComponent>() }
			, m_cameraList{ a_wdp.get_old_entity_view_list(*this, CameraComponents{}) }
		{}

		void update() const
		{
			auto const t_camera = m_cameraList.find(
				m_directorComponent.m_currentCamera
			);

			if (t_camera == nullptr && !m_cameraList.empty())
			{
				m_directorComponent.m_currentCamera = m_cameraList.front().get_id();
			}
		}

		// Attributes
		DirectorComponent& m_directorComponent;
		_aoecs::entity_view_list<
			TransformComponent const
			, CameraComponent const
		> const& m_cameraList;
	};
}
