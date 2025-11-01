#pragma once

#include <vob/aoe/rendering/GraphicTypes.h>


namespace vob::aoegl
{
	struct RenderTexture
	{
		GraphicId texture = 0;
		GraphicId framebuffer = 0;
		GraphicId renderbuffer = 0;
	};
}
