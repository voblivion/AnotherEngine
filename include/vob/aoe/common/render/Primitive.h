#pragma once

#include <array>

namespace vob::aoe::common
{
	using Index = std::uint32_t;
	
	struct Line
	{
		std::uint32_t m_v0;
		std::uint32_t m_v1;
	};

	struct Triangle
	{
		std::uint32_t m_v0;
		std::uint32_t m_v1;
		std::uint32_t m_v2;
	};
}
