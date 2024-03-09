#pragma once

#include <vob/aoe/rendering/components/camera_component.h>

#include <vob/aoe/spacetime/transform.h>

#include <glm/glm.hpp>

#include <numbers>
#include <tuple>


namespace vob::aoegl
{
	inline auto get_camera_view_projection_transform(
		aoest::position const& a_position,
		aoest::rotation const& a_rotation,
		camera_component const& a_camera,
		float a_aspectRatio)
	{
		auto const transform = aoest::combine(a_position, a_rotation);
		auto const viewTransform = glm::perspective(
			a_camera.m_fov,
			a_aspectRatio,
			a_camera.m_nearClip,
			a_camera.m_farClip);
		auto const invTransform = glm::inverse(transform);
		auto const viewProjectionTransform = viewTransform * invTransform;
		return viewProjectionTransform;
	}

	template <typename TDirectorWorldComponent, typename TCameraEntities>
	auto get_active_camera_settings(
		TDirectorWorldComponent const& a_directorWorldComponent,
		TCameraEntities const& a_cameraEntities,
		float a_aspectRatio)
	{
		auto const cameraEntityIt = a_cameraEntities.find(a_directorWorldComponent.m_activeCamera);
		if (cameraEntityIt == a_cameraEntities.end())
		{
			auto const position = aoest::position{ 0.0f };
			auto const rotation = aoest::rotation{};
			auto const camera = camera_component{};
			return std::make_tuple(
				position, get_camera_view_projection_transform(position, rotation, camera, a_aspectRatio));
		}

		auto const [position, rotation, camera] = a_cameraEntities.get(*cameraEntityIt);
		return std::make_tuple(
			position,
			get_camera_view_projection_transform(position, rotation, camera, a_aspectRatio));
	}
}
