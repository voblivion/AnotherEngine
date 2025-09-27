#pragma once

#include <glm/glm.hpp>


namespace vob::aoeph
{
	struct aabb
	{
		glm::vec3 min;
		glm::vec3 max;
	};

	struct triangle
	{
		glm::vec3 p0;
		glm::vec3 p1;
		glm::vec3 p2;
	};

	struct segment
	{
		glm::vec3 p0;
		glm::vec3 p1;
	};

	struct plane
	{
		glm::vec3 point;
		glm::vec3 normal;
	};
}
