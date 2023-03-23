#pragma once

#include <vob/aoe/rendering/graphic_types.h>

namespace vob::aoegl
{
	struct debug_mesh
	{
		graphic_id m_vao = 0;
		graphic_id m_vbo = 0;
		graphic_id m_ebo = 0;
		graphic_size m_lineCount = 0;
	};
}
