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

	inline auto getActiveCameraUniforms(
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

	inline auto computeCameraFrustumPlanes(glm::vec3 const& a_cameraPos, glm::quat const& a_cameraRot, CameraComponent const& a_cameraComponent, float a_aspectRatio)
	{
		auto const cameraForward = a_cameraRot * glm::vec3{ 0.0f, 0.0f, -1.0f };
		auto const cameraRight = a_cameraRot * glm::vec3{ 1.0f, 0.0f, 0.0f };
		auto const cameraUp = a_cameraRot * glm::vec3{ 0.0f, 1.0f, 0.0f };
		auto const nearCenter = a_cameraPos + cameraForward * a_cameraComponent.nearClip;
		auto const farCenter = a_cameraPos + cameraForward * a_cameraComponent.farClip;
		auto const tanHalfFov = std::tan(a_cameraComponent.fov * 0.5f);
		auto const nearHalfHeight = a_cameraComponent.nearClip * tanHalfFov;
		auto const nearHalfWidth = nearHalfHeight * a_aspectRatio;
		auto const farHalfHeight = a_cameraComponent.farClip * tanHalfFov;
		auto const farHalfWidth = farHalfHeight * a_aspectRatio;

		auto const leftPlaneNormal = glm::normalize(glm::cross(nearCenter - cameraRight * nearHalfWidth - a_cameraPos, cameraUp));
		auto const rightPlaneNormal = glm::normalize(glm::cross(cameraUp, nearCenter + cameraRight * nearHalfWidth - a_cameraPos));
		auto const upPlaneNormal = glm::normalize(glm::cross(nearCenter + cameraUp * nearHalfHeight - a_cameraPos, cameraRight));
		auto const downPlaneNormal = glm::normalize(glm::cross(cameraRight, nearCenter - cameraUp * nearHalfHeight - a_cameraPos));

		return std::array{
			std::pair{cameraForward, -glm::dot(cameraForward, nearCenter)},
			std::pair{-cameraForward, -glm::dot(-cameraForward, farCenter)},
			std::pair{leftPlaneNormal, -glm::dot(leftPlaneNormal, a_cameraPos)},
			std::pair{rightPlaneNormal, -glm::dot(rightPlaneNormal, a_cameraPos)},
			std::pair{upPlaneNormal, -glm::dot(upPlaneNormal, a_cameraPos)},
			std::pair{downPlaneNormal, -glm::dot(downPlaneNormal, a_cameraPos)}
		};
	}

	inline bool testSphereCameraFrustumPlanesIntersection(
		std::array<std::pair<glm::vec3, float>, 6> const& a_cameraFrustumPlanes, glm::vec3 const& a_spherePos, float a_sphereRadius)
	{
		for (auto const& plane : a_cameraFrustumPlanes)
		{
			if (glm::dot(plane.first, a_spherePos) + plane.second < -a_sphereRadius)
			{
				return false;
			}
		}

		return true;
	}
}
