#pragma once

#include "vertex.h"
#include "types.h"

#include <vector>


namespace vob::aoegl
{
	struct render_texture
	{
		graphic_id m_textureId;
		graphic_id m_framebufferId;
		graphic_id m_renderbufferId;
		glm::ivec2 m_size;
	};

	struct texture
	{
		graphic_id m_id;
	};

	struct mesh
	{
		graphic_id m_vao;
		graphic_id m_vbo;
		graphic_id m_ebo;
		graphic_size m_triangleVertexCount;
	};
}
