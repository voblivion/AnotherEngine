#pragma once

#include <vob/aoe/rendering/data/vertex_data.h>
#include <vob/aoe/rendering/primitives.h>

#include <string>
#include <vector>


namespace vob::aoegl
{
#ifdef _TODO_ANIMATION_
	struct vertex_weight
	{
		std::uint32_t m_vertexIndex;
		float m_weight;
	};

	struct mesh_joint
	{
		std::pmr::string m_name;
		glm::mat4 m_reverseBindTransform;
		std::pmr::vector<vertex_weight> m_vertexWeights;
	};
#endif

	struct mesh_data
	{
		std::pmr::vector<glm::vec3> m_positions;
		std::pmr::vector<glm::vec2> m_textureCoords;
		std::pmr::vector<glm::vec3> m_normals;
		std::pmr::vector<glm::vec3> m_tangents;

#ifdef _TODO_ANIMATION_
		std::pmr::vector<mesh_joint> m_joints;
#endif

		std::pmr::vector<triangle> m_triangles;
	};
}
