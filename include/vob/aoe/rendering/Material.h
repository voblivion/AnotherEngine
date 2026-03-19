#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"

#include "vob/misc/std/enum_traits.h"

#include <array>


namespace vob::aoegl
{
	struct Material
	{
		GraphicId paramsUbo;
		std::vector<GraphicId> textureIds;
	};
}
