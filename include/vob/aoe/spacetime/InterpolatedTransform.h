#pragma once

#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>

#include <array>


namespace vob::aoest
{
	struct InterpolatedPosition
	{
		std::array<glm::vec3, 4> positions;
		int32_t endIndex;
		
		glm::vec3 source;
		glm::vec3 target;
	};

	struct InterpolatedRotation
	{
		std::array<glm::quat, 4> rotations;
		int32_t endIndex;
		glm::quat source;
		glm::quat target;
	};
}
