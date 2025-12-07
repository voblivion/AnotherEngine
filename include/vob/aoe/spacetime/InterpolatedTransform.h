#pragma once

#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>


namespace vob::aoest
{
	struct InterpolatedPosition
	{
		glm::vec3 source;
		glm::vec3 target;
	};

	struct InterpolatedRotation
	{
		glm::quat source;
		glm::quat target;
	};
}
