#pragma once

#include <vob/aoe/rendering/GraphicTypes.h>

#include "vob/aoe/rendering/resources/GpuResource.h"
#include "vob/aoe/rendering/shaders/defines.h"

#include <cstdint>
#include <vector>


namespace vob::aoegl
{
	struct ModelTransformComponent
	{
		UniformModelParams prevModelParams;
		GpuBuffer modelParamsUbo;
		float boundingRadius = 0.0f;
	};
}
