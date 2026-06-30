#pragma once

#include <vob/aoe/rendering/GraphicTypes.h>

#include "vob/aoe/rendering/resources/GpuBuffer.h"
#include "vob/aoe/rendering/resources/Handle.h"
#include "vob/aoe/rendering/shaders/defines.h"

#include <cstdint>
#include <vector>


namespace vob::aoegl
{
	struct ModelTransformComponent
	{
		UniformModelParams prevModelParams;
		GraphicId modelParamsUbo = k_invalidId;
		Handle<GpuBuffer> modelParams;
		float boundingRadius = 0.0f;
	};
}
