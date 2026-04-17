#pragma once

#include "vob/aoe/spacetime/PositionComponent.h"
#include "vob/aoe/spacetime/RotationComponent.h"

#include "vob/aoe/debug/Check.h"

#include "glm/glm.hpp"


namespace vob::aoest
{
	inline glm::mat4 combine(glm::vec3 const& a_position, glm::quat const& a_rotation)
	{
		auto matrix = glm::mat4_cast(a_rotation);
		matrix[3] = glm::vec4(a_position, 1.0f);
		return matrix;
	}

	inline glm::mat4 combine(PositionComponent const& a_positionCmp, RotationComponent const& a_rotationCmp)
	{
		return combine(a_positionCmp.value, a_rotationCmp.value);
	}

	inline glm::vec3 transformPositionSkewed(glm::mat4 const& a_transform, glm::vec3 const& a_position)
	{
		auto const positionHeterogenous = a_transform * glm::vec4{ a_position, 1.0f };
		return glm::vec3{ positionHeterogenous } / positionHeterogenous.w;
	}

	inline glm::vec3 transformPosition(glm::mat4 const& a_transform, glm::vec3 const& a_position)
	{
		VOB_AOE_CHECK_TERMINATE_SLOW(glm::epsilonEqual((a_transform * glm::vec4{ a_position, 1.0f }).w, 1.0f, 0.01f), "Transform is not affine. Use TransformPositionSkewed.");
		return glm::vec3(a_transform * glm::vec4{ a_position, 1.0f });
	}

	inline glm::vec3 transformPosition(glm::vec3 const& a_transformPosition, glm::quat const& a_transformRotation, glm::vec3 const& a_position)
	{
		return a_transformPosition + a_transformRotation * a_position;
	}

	inline glm::vec3 transformPosition(PositionComponent const& a_positionCmp, RotationComponent const& a_rotationCmp, glm::vec3 const& a_position)
	{
		return a_positionCmp.value + a_rotationCmp.value * a_position;
	}

	inline glm::vec3 transformDirection(glm::mat4 const& a_transform, glm::vec3 const& a_direction)
	{
		return glm::vec3(a_transform * glm::vec4{ a_direction, 0.0f });
	}
}
