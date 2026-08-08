#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"
#include "vob/aoe/rendering/shaders/defines.h"

#include <glm/glm.hpp>

#include <string_view>
#include <vector>


namespace vob::aoegl
{
	struct DebugRenderInspectorContext
	{
		std::string_view selectedName;
		std::vector<std::string_view> names;
		float exposure = 1.0f;

		GraphicId capturedTexture = k_invalidId;
		glm::ivec2 capturedResolution{ 0 };
		GraphicEnum capturedInternalFormat = 0;
		DebugType capturedType = DebugType::ColorTexture;
		glm::vec2 capturedDepthRange{ 0.0f };
	};
}
