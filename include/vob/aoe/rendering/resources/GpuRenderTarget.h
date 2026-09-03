#pragma once

#include "vob/aoe/rendering/resources/GlResource.h"

#include <glm/glm.hpp>


namespace vob::aoegl
{
	struct GpuRenderTarget
	{
		GlFramebuffer framebuffer;
		glm::ivec2 resolution;
	};
}
