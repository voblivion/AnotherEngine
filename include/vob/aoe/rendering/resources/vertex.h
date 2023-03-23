#pragma once

#include <vob/aoe/rendering/graphic_types.h>

#include <vob/aoe/spacetime/measures.h>


namespace vob::aoegl
{
	struct post_process_vertex
	{
		glm::vec2 m_position;
		glm::vec2 m_textureCoord;
	};
}
