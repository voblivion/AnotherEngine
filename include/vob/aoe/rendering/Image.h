#pragma once

#include <glm/glm.hpp>

#include <vector>


namespace vob::aoegl
{
	struct Image
	{
		int32_t channelCount;
		glm::uvec2 size;
		std::pmr::vector<uint8_t> data;
	};
}
