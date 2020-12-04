#pragma once
#include <vob/aoe/core/ecs/WorldDataProvider.h>

#include <vob/aoe/common/render/CameraComponent.h>
#include <vob/aoe/common/space/TransformComponent.h>
#include <vob/aoe/common/render/DirectorComponent.h>
#include <vob/aoe/common/window/WindowComponent.h>

namespace vob::aoe::common
{
	template <typename TSceneShaderProgram>
	bool initSceneShaderProgram(
		TSceneShaderProgram const& a_sceneShaderProgram
		, WindowComponent const& a_windowComponent
		, DirectorComponent const& a_directorComponent
		, ecs::EntityList<TransformComponent const, CameraComponent const> const& a_cameramanEntityList
	)
	{
		if (!a_sceneShaderProgram.isReady())
		{
			return false;
		}

		a_sceneShaderProgram.use();

		auto const cameramanEntity = a_cameramanEntityList.find(a_directorComponent.m_currentCamera);
		if (cameramanEntity == nullptr)
		{
			return false;
		}
		auto const& cameramanTransformComponent = cameramanEntity->getComponent<TransformComponent>();
		auto const& cameramanCameraComponent = cameramanEntity->getComponent<CameraComponent>();

		auto const viewMatrix = cameramanTransformComponent.m_matrix;
		a_sceneShaderProgram.setUniform(
			a_sceneShaderProgram.getViewUniformLocation()
			, glm::inverse(viewMatrix)
		);

		auto const windowSize = a_windowComponent.getWindow().getSize();
		auto const projectionMatrix = glm::perspective(
			glm::radians(cameramanCameraComponent.fov)
			, static_cast<float>(windowSize.x) / windowSize.y
			, cameramanCameraComponent.nearClip
			, cameramanCameraComponent.farClip
		);
		a_sceneShaderProgram.setUniform(
			a_sceneShaderProgram.getProjectionUniformLocation()
			, projectionMatrix
		);
		a_sceneShaderProgram.setUniform(
			a_sceneShaderProgram.getViewPositionUniformLocation()
			, getTranslation(cameramanTransformComponent.m_matrix)
		);
		return true;
	}
}
