#pragma once

#include <glm/glm.hpp>

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
		std::pmr::vector<Level> levels;
		std::pmr::vector<uint8_t> data;
	};
}
