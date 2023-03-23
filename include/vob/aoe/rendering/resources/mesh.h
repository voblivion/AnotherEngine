#pragma once

#include <vob/aoe/rendering/graphic_types.h>

#include <array>


namespace vob::aoegl
{
	struct mesh
	{
		graphic_id m_positionVbo = 0;
		graphic_id m_textureCoordVbo = 0;
		graphic_id m_normalVbo = 0;
		graphic_id m_tangentVbo = 0;
#ifdef _TODO_ANIMATION_
		graphic_id m_jointIndicesVbo = 0;
		graphic_id m_jointWeightsVbo = 0;
#endif

		graphic_id m_ebo = 0;
		graphic_size m_triangleCount = 0;

#ifdef _TODO_ANIMATION_
		std::pmr::vector<glm::mat4> m_rigPose;
#endif
	};
}
