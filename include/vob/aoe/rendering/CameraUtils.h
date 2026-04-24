#pragma once

#include "vob/aoe/rendering/components/CameraComponent.h"

#include "vob/aoe/spacetime/PositionComponent.h"
#include "vob/aoe/spacetime/RotationComponent.h"

#include "entt/entt.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

#include <array>
#include <numbers>
#include <utility>


namespace vob::aoegl
{
	struct CameraProperties
	{
		glm::vec3 position;
		glm::quat rotation;
		float nearClip;
		float farClip;
		float fov;
	};

	inline CameraProperties getCameraProperties(
		entt::view<entt::get_t<aoest::PositionComponent const, aoest::RotationComponent const, CameraComponent const>> a_cameraEntities,
		entt::entity a_cameraEntity)
	{
		if (!a_cameraEntities.contains(a_cameraEntity))
		{
			return CameraProperties{
				.position = glm::vec3{ 0.0f },
				.rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f },
				.nearClip = 0.1f,
				.farClip = 1000.0f,
				.fov = 70.0f * std::numbers::pi_v<float> / 360.0f
			};
		}

		auto const [positionCmp, rotationCmp, cameraCmp] = a_cameraEntities.get(a_cameraEntity);
		return CameraProperties{
			.position = positionCmp.value,
			.rotation = rotationCmp.value,
			.nearClip = cameraCmp.nearClip,
			.farClip = cameraCmp.farClip,
			.fov = cameraCmp.fov
		};
	}

	inline glm::vec3 getFocusPosition(
		entt::view<entt::get_t<aoest::PositionComponent const>> a_focusEntities, entt::entity a_focusEntity)
	{
		if (!a_focusEntities.contains(a_focusEntity))
		{
			return glm::vec3{ 0.0f };
		}

		auto const [positionCmp] = a_focusEntities.get(a_focusEntity);
		return positionCmp.value;
	}

	using ViewFrustumPlanes = std::array<glm::vec4, 6>;

	inline ViewFrustumPlanes computeViewFrustumPlanes(glm::mat4 a_worldToClip)
	{
		// Gribb-Harman algorithm.
		auto const normalizeGribbHartmann = [](glm::vec4 const& value)
			{
				auto const len = glm::length(glm::vec3{ value });
				return glm::vec4{ glm::vec3{value} / len, value.w / len };
			};
		auto const m = glm::transpose(a_worldToClip);
		return {
			normalizeGribbHartmann(m[3] + m[0]),
			normalizeGribbHartmann(m[3] - m[0]),
			normalizeGribbHartmann(m[3] + m[1]),
			normalizeGribbHartmann(m[3] - m[1]),
			normalizeGribbHartmann(m[3] + m[2]),
			normalizeGribbHartmann(m[3] - m[2])
		};
	}

	inline bool testViewFrustumPlanes(ViewFrustumPlanes const& a_viewFrustumPlanes, glm::vec3 const& a_position, float a_radius)
	{
		return std::all_of(
			a_viewFrustumPlanes.begin(),
			a_viewFrustumPlanes.end(),
			[&a_position, a_radius](glm::vec4 const& a_plane)
			{
				return glm::dot(glm::vec3{ a_plane }, a_position) + a_plane.w >= -a_radius;
			});
	}
}
