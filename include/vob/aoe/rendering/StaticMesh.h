#pragma once

#include <glm/glm.hpp>

#include <vector>


namespace vob::aoegl
{
	struct StaticVertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec3 tangent;
	};

	struct StaticMesh
	{
		struct Part
		{
			std::pmr::vector<StaticVertex> vertices;
			std::pmr::vector<uint32_t> indices;
		};

		std::pmr::vector<Part> parts;
	};
}
