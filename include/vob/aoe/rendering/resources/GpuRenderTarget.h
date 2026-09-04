#pragma once

#include "vob/aoe/rendering/resources/GpuResource.h"

#include <glm/glm.hpp>


namespace vob::aoegl
{
	struct GpuRenderTarget
	{
		GpuFramebuffer framebuffer;
		glm::ivec2 resolution;
	};
}
