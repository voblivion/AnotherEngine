#pragma once

#include <vob/aoe/rendering/graphic_types.h>

namespace vob::aoegl
{
	struct render_texture
	{
		graphic_id m_texture = 0;
		graphic_id m_framebuffer = 0;
		graphic_id m_renderbuffer = 0;
	};
}
