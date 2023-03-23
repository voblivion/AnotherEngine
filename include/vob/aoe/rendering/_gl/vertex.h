#pragma once

#include "types.h"

#include <glm/glm.hpp>


namespace vob::aoegl
{
	struct debug_vertex_data
	{
		glm::vec3 m_position;
		glm::vec4 m_color;
	};

	struct static_vertex_data
	{
		glm::vec3 m_position;
		glm::vec2 m_textureCoords;
		glm::vec3 m_normal;
		glm::vec3 m_tangent;
	};

	struct rigged_vertex_data : public static_vertex_data
	{
		glm::u8vec4 m_boneIndices;
		glm::vec4 m_boneWeights;
	};
}
