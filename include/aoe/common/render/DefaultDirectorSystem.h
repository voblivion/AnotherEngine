#pragma once
#include <aoe/common/render/CameraComponent.h>
#include <aoe/common/render/DirectorComponent.h>
#include <aoe/core/ecs/WorldDataProvider.h>


namespace aoe::common
{
	struct DefaultDirectorSystem
	{
		using CameraComponents = ecs::ComponentTypeList<
			TransformComponent const, CameraComponent const>;

		// Constructor
		explicit DefaultDirectorSystem(ecs::WorldDataProvider& a_wdp)
			: m_directorComponent{ *a_wdp.getWorldComponent<DirectorComponent>() }
			, m_cameraList{ a_wdp.getEntityList(*this, CameraComponents{}) }
		{}

		void update() const
		{
			auto const t_camera = m_cameraList.find(
				m_directorComponent.m_currentCamera
			);

			if (t_camera == nullptr && !m_cameraList.empty())
			{
				m_directorComponent.m_currentCamera = m_cameraList.front().getId();
			}
		}

		// Attributes
		DirectorComponent& m_directorComponent;
		ecs::SystemEntityList<DefaultDirectorSystem
			, TransformComponent const, CameraComponent const> const& m_cameraList;
	};
}
