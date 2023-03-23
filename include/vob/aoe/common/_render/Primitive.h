#pragma once

#include <array>

namespace vob::aoegl
{
	struct old_line
	{
		std::uint32_t m_v0;
		std::uint32_t m_v1;
	};

	struct old_triangle
	{
		std::uint32_t m_v0;
		std::uint32_t m_v1;
		std::uint32_t m_v2;
	};
}

namespace vob::aoe::common
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
