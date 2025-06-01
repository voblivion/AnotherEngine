#pragma once

#include <vob/misc/physics/measure.h>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_CXX17
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

	inline auto to_rgba(const std::uint32_t a_hex)
	{
		return rgba{
			color_channel{static_cast<float>((a_hex & 0xff000000) >> 24) / 256.0f},
			color_channel{static_cast<float>((a_hex & 0x00ff0000) >> 16) / 256.0f},
			color_channel{static_cast<float>((a_hex & 0x0000ff00) >> 8) / 256.0f},
			color_channel{static_cast<float>((a_hex & 0x000000ff) >> 0) / 256.0f},
		};
	}

#pragma message(VOB_MISTD_TODO "glm cannot use constexpr...")
	static const rgba k_white = rgba{ 1.0f, 1.0f, 1.0f, 1.0f };
	static const rgba k_gray = rgba{ 0.5f, 0.5f, 0.5f, 1.0f };
	static const rgba k_black = rgba{ 0.0f, 0.0f, 0.0f, 1.0f };
	static const rgba k_red = rgba{ 1.0f, 0.0f, 0.0f, 1.0f };
	static const rgba k_maroon = rgba{ 0.5f, 0.0f, 0.0f, 1.0f };
	static const rgba k_green = rgba{ 0.0f, 1.0f, 0.0f, 1.0f };
	static const rgba k_forest = rgba{ 0.0f, 0.5f, 0.0f, 1.0f };
	static const rgba k_blue = rgba{ 0.0f, 0.0f, 1.0f, 1.0f };
	static const rgba k_navy = rgba{ 0.0f, 0.0f, 0.5f, 1.0f };
	static const rgba k_blueprint = to_rgba(0x14252e);
	static const rgba k_yellow = rgba{ 1.0f, 1.0f, 0.0f, 1.0f };
	static const rgba k_olive = rgba{ 0.5f, 0.5f, 0.0f, 1.0f };
	static const rgba k_magenta = rgba{ 1.0f, 0.0f, 1.0f, 1.0f };
	static const rgba k_eggplant = rgba{ 0.5f, 0.0f, 0.5f, 1.0f };
	static const rgba k_cyan = rgba{ 0.0f, 1.0f, 1.0f, 1.0f };
	static const rgba k_teal = rgba{ 0.0f, 0.5f, 0.5f, 1.0f };
	static const rgba k_orange = rgba{ 1.0f, 0.5f, 0.0f, 1.0f };
	static const rgba k_rose = rgba{ 1.0f, 0.0f, 0.5f, 1.0f };
	static const rgba k_chartreuse = rgba{ 0.5f, 1.0f, 0.0f, 1.0f };
	static const rgba k_spring = rgba{ 0.0f, 1.0f, 0.5f, 1.0f };
	static const rgba k_violet = rgba{ 0.5f, 0.0f, 1.0f, 1.0f };
	static const rgba k_azure = rgba{ 0.0f, 0.5f, 1.0f, 1.0f };

	inline auto to_rgb(glm::vec3 const& a_vector)
	{
		return rgb{
			color_channel{a_vector.x},
			color_channel{a_vector.y},
			color_channel{a_vector.z} };
	}

	inline auto to_rgba(glm::vec3 const& a_vector)
	{
		return rgba{
			color_channel{a_vector.x},
			color_channel{a_vector.y},
			color_channel{a_vector.z},
			1.0f };
	}

	inline auto to_rgb(glm::vec4 const& a_vector)
	{
		return rgb{
			color_channel{a_vector.x},
			color_channel{a_vector.y},
			color_channel{a_vector.z} };
	}

	inline auto to_rgba(glm::vec4 const& a_vector)
	{
		return rgba{
			color_channel{a_vector.x},
			color_channel{a_vector.y},
			color_channel{a_vector.z},
			color_channel{ a_vector.w } };
	}
}
