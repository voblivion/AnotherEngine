#pragma once

#include "vob/aoe/rendering/resources/GpuShader.h"
#include "vob/aoe/rendering/resources/Handle.h"

#include "vob/misc/std/bounded_vector.h"
#include "vob/misc/std/container_util.h"

#include "vob/aoe/rendering/shaders/defines.h"

#include <memory>


namespace vob::aoegl
{
	struct GpuMaterial
	{
		Handle<GpuShader> shader;
		GraphicId paramsUbo;
		mistd::bounded_vector<GraphicId, k_materialTexturesCapacity> textureIds;
	};
}
