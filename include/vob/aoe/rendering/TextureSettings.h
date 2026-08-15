#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"

#include <array>


namespace vob::aoegl
{
	struct TextureSettings
	{
		enum class ColorSpace
		{
			Linear,
			Srgb
		};

		enum class SamplerType
		{
			Simple,
			Array,
			Cube
		};

		ColorSpace colorSpace = ColorSpace::Linear;
		SamplerType samplerType = SamplerType::Simple;
		std::array<GraphicEnum, 4> swizzle = { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
	};
}
