#pragma once

#include <vob/aoe/rendering/GraphicTypes.h>

#include "vob/aoe/rendering/shaders/defines.h"

#include <cstdint>
#include <vector>


namespace vob::aoegl
{
	struct ModelTransformComponent
	{
		UniformModelParams modelParams;
		GraphicId modelParamsUbo = k_invalidId;
		float boundingRadius = 0.0f;
	};
}
