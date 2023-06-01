#pragma once

#include <vob/misc/physics/measure.h>

#include <glm/glm.hpp>

#include <vob/misc/std/message_macros.h>


namespace vob::aoegl
{
	struct color_channel
	{
		float m_value;

		operator float() const
		{
			return m_value;
		}
	};

	template <std::size_t t_channels>
	using color = glm::vec<t_channels, color_channel>;

	using rgb = color<3>;
	using rgba = color<4>;

#pragma message(VOB_MISTD_TODO "glm cannot use constexpr...")
	static const rgba k_white = rgba{ 1.0f, 1.0f, 1.0f, 1.0f };
	static const rgba k_black = rgba{ 0.0f, 0.0f, 0.0f, 1.0f };
	static const rgba k_red = rgba{ 1.0f, 0.0f, 0.0f, 1.0f };
	static const rgba k_green = rgba{ 0.0f, 1.0f, 0.0f, 1.0f };
	static const rgba k_blue = rgba{ 0.0f, 0.0f, 1.0f, 1.0f };
	static const rgba k_yellow = rgba{ 1.0f, 1.0f, 0.0f, 1.0f };
	static const rgba k_purple = rgba{ 1.0f, 0.0f, 1.0f, 1.0f };
	static const rgba k_cyan = rgba{ 0.0f, 1.0f, 1.0f, 1.0f };
	static const rgba k_dark_blue = rgba{ 0.0f, 0.0f, 0.5f, 1.0f };
}
