#pragma once

#include "vob/aoe/rendering/TextureSettings.h"

#include <glm/glm.hpp>

#include <optional>
#include <vector>


namespace vob::aoegl
{
	struct ImageData
	{
		enum class Format
		{
			R8,
			Rg8,
			Rgb8,
			Rgba8,
			Bc1,
			Bc1a,
			Bc4,
			Bc5,
			Bc6h,
			Bc7
		};

		struct Level
		{
			glm::uvec2 size;
			size_t dataOffset;
			size_t dataSize;
		};

		Format format;
		std::optional<TextureSettings::ColorSpace> colorSpace;
		std::pmr::vector<Level> levels;
		std::pmr::vector<uint8_t> data;
	};
}
