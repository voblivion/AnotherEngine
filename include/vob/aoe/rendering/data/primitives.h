#pragma once

#include <cinttypes>


namespace vob::aoegl
{
	struct line
	{
		std::uint32_t m_v0;
		std::uint32_t m_v1;
	};

	struct triangle
	{
		std::uint32_t m_v0;
		std::uint32_t m_v1;
		std::uint32_t m_v2;
	};
}
