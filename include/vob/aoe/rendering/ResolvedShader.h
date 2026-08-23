#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"


namespace vob::aoegl
{
	struct ResolvedShader
	{
		GraphicId program = k_invalidId;
		GraphicId depthProgram = k_invalidId;
		GraphicId shadowMapProgram = k_invalidId;
		bool isAlphaMasked = false;

		friend bool operator==(ResolvedShader const&, ResolvedShader const&) = default;
	};
}
