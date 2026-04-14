#pragma once

#include <vob/aoe/rendering/CameraComponent.h>
#include <vob/aoe/rendering/CameraDirectorContext.h>

#include "vob/aoe/spacetime/PositionComponent.h"
#include "vob/aoe/spacetime/RotationComponent.h"
#include "vob/aoe/spacetime/TransformUtils.h"

#include <entt/entt.hpp>
#include <glm/glm.hpp>


namespace vob::aoegl
{
	inline auto getCameraViewProjectionTransform(
		aoest::PositionComponent const& a_positionCmp,
		aoest::RotationComponent const& a_rotationCmp,
		CameraComponent const& a_cameraCmp,
		float a_aspectRatio)
	{
		auto const viewTransform = glm::perspective(
			a_cameraCmp.fov,
			a_aspectRatio,
			a_cameraCmp.nearClip,
			a_cameraCmp.farClip);

		auto const transform = aoest::combine(a_positionCmp.value, a_rotationCmp.value);
		auto const transformInv = glm::inverse(transform);

		return viewTransform * transformInv;
	}

	inline auto getActiveCameraUniforms(
		CameraDirectorContext const& a_cameraDirectorContext,
		entt::view<entt::get_t<aoest::PositionComponent const, aoest::RotationComponent const, CameraComponent const>> const& a_cameraEntities,
		float a_aspectRatio)
	{
		if (!a_cameraEntities.contains(a_cameraDirectorContext.activeCameraEntity))
		{
			auto const positionCmp = aoest::PositionComponent{ .value = glm::vec3{ 0.0f } };
			auto const rotationCmp = aoest::RotationComponent{};
			auto const cameraComponent = CameraComponent{};
			return std::make_tuple(positionCmp.value, getCameraViewProjectionTransform(positionCmp, rotationCmp, cameraComponent, a_aspectRatio));
		}

		auto const [positionCmp, rotationCmp, cameraComponent] = a_cameraEntities.get(a_cameraDirectorContext.activeCameraEntity);
		return std::make_tuple(positionCmp.value, getCameraViewProjectionTransform(positionCmp, rotationCmp, cameraComponent, a_aspectRatio));
	}

	inline auto computeCameraFrustumPlanes(glm::vec3 const& a_cameraPos, glm::quat const& a_cameraRot, CameraComponent const& a_cameraCmp, float a_aspectRatio)
	{
		auto const cameraForward = a_cameraRot * glm::vec3{ 0.0f, 0.0f, -1.0f };
		auto const cameraRight = a_cameraRot * glm::vec3{ 1.0f, 0.0f, 0.0f };
		auto const cameraUp = a_cameraRot * glm::vec3{ 0.0f, 1.0f, 0.0f };
		auto const nearCenter = a_cameraPos + cameraForward * a_cameraCmp.nearClip;
		auto const farCenter = a_cameraPos + cameraForward * a_cameraCmp.farClip;
		auto const tanHalfFov = std::tan(a_cameraCmp.fov * 0.5f);
		auto const nearHalfHeight = a_cameraCmp.nearClip * tanHalfFov;
		auto const nearHalfWidth = nearHalfHeight * a_aspectRatio;

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
