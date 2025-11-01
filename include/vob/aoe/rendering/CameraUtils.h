#pragma once

#include <vob/aoe/rendering/CameraComponent.h>
#include <vob/aoe/rendering/CameraDirectorContext.h>

#include <vob/aoe/spacetime/Transform.h>

#include <entt/entt.hpp>
#include <glm/glm.hpp>


namespace vob::aoegl
{
	inline auto getCameraViewProjectionTransform(
		aoest::Position const& a_position,
		aoest::Rotation const& a_rotation,
		CameraComponent const& a_cameraComponent,
		float a_aspectRatio)
	{
		auto const viewTransform = glm::perspective(
			a_cameraComponent.fov,
			a_aspectRatio,
			a_cameraComponent.nearClip,
			a_cameraComponent.farClip);

		auto const transform = aoest::combine(a_position, a_rotation);
		auto const transformInv = glm::inverse(transform);

		return viewTransform * transformInv;
	}

	auto getActiveCameraSettings(
		CameraDirectorContext const& a_cameraDirectorContext,
		entt::view<entt::get_t<aoest::Position const, aoest::Rotation const, CameraComponent const>> const& a_cameraEntities,
		float a_aspectRatio)
	{
		if (!a_cameraEntities.contains(a_cameraDirectorContext.activeCameraEntity))
		{
			auto const position = aoest::Position{ 0.0f };
			auto const rotation = aoest::Rotation{};
			auto const cameraComponent = CameraComponent{};
			return std::make_tuple(position, getCameraViewProjectionTransform(position, rotation, cameraComponent, a_aspectRatio));
		}

		auto const [position, rotation, cameraComponent] = a_cameraEntities.get(a_cameraDirectorContext.activeCameraEntity);
		return std::make_tuple(position, getCameraViewProjectionTransform(position, rotation, cameraComponent, a_aspectRatio));
	}
}
