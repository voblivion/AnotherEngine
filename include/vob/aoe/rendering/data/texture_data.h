#pragma once

#include <glm/glm.hpp>

#include <cassert>
#include <vector>


namespace vob::aoegl
{
	struct texture_data
	{
		std::size_t m_channelCount;
		glm::uvec2 m_size;
		std::pmr::vector<std::uint8_t> m_data;
	};
}
