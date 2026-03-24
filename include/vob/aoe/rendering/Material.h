#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"

#include "vob/misc/std/enum_traits.h"
#include "vob/misc/std/bounded_vector.h"

#include "vob/aoe/rendering/shaders/defines.h"

#include <array>


namespace vob::aoegl
{
	struct Material
	{
		GraphicId paramsUbo;
		mistd::bounded_vector<GraphicId, k_materialTexturesCapacity> textureIds;
	};
}
