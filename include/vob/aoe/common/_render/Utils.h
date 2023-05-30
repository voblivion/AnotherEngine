#pragma once
#include <vob/aoe/ecs/world_data_provider.h>

#include <vob/aoe/common/_render/Cameracomponent.h>
#include <vob/aoe/common/space/Transformcomponent.h>
#include <vob/aoe/common/_render/Directorcomponent.h>
#include <vob/aoe/common/window/WorldWindowcomponent.h>

namespace vob::aoe::common
{
	template <typename TSceneShaderProgram>
	bool initSceneShaderProgram(
		TSceneShaderProgram const& a_sceneShaderProgram
		, WorldWindowComponent const& a_worldWindowComponent
		, DirectorComponent const& a_directorComponent
		, aoecs::entity_map_observer_list_ref<TransformComponent const, CameraComponent const> const& a_cameramanEntityList
	)
    {
		if (!a_sceneShaderProgram.isReady())
		{
			return false;
		}

		a_sceneShaderProgram.use();

		auto const cameramanEntity = a_cameramanEntityList.find(a_directorComponent.m_currentCamera);
		if (cameramanEntity == a_cameramanEntityList.end())
		{
			return false;
		}
		auto const& transformComponent = cameramanEntity->get<TransformComponent>();
		auto const& cameraComponent = cameramanEntity->get<CameraComponent>();

		auto const viewMatrix = transformComponent.m_matrix;
		a_sceneShaderProgram.setUniform(
			a_sceneShaderProgram.getViewUniformLocation()
			, glm::inverse(viewMatrix)
		);

        const auto& window = a_worldWindowComponent.getWindow();
        auto const windowSize = window.getSize();
		auto const projectionMatrix = glm::perspective(
			glm::radians(cameraComponent.fov)
			, static_cast<float>(windowSize.x) / windowSize.y
			, cameraComponent.nearClip
			, cameraComponent.farClip
		);
		a_sceneShaderProgram.setUniform(
			a_sceneShaderProgram.getProjectionUniformLocation()
			, projectionMatrix
		);
		a_sceneShaderProgram.setUniform(
			a_sceneShaderProgram.getViewPositionUniformLocation()
			, getTranslation(transformComponent.m_matrix)
		);
		return true;
	}
}
