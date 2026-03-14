#pragma once

#include <glm/glm.hpp>


namespace vob::aoegl
{
	struct ColorChannel
	{
		float value;

		operator float() const
		{
			return value;
		}
	};

	template <std::size_t t_channels>
	using Color = glm::vec<t_channels, ColorChannel>;

	using Rgb = Color<3>;
	using Rgba = Color<4>;

	inline auto toRgba(const std::uint32_t a_hex)
	{
		return Rgba{
			ColorChannel{static_cast<float>((a_hex & 0xff000000) >> 24) / 256.0f},
			ColorChannel{static_cast<float>((a_hex & 0x00ff0000) >> 16) / 256.0f},
			ColorChannel{static_cast<float>((a_hex & 0x0000ff00) >> 8) / 256.0f},
			ColorChannel{static_cast<float>((a_hex & 0x000000ff) >> 0) / 256.0f},
		};
	}

	static const Rgba k_white = Rgba{ 1.0f, 1.0f, 1.0f, 1.0f };
	static const Rgba k_gray = Rgba{ 0.5f, 0.5f, 0.5f, 1.0f };
	static const Rgba k_black = Rgba{ 0.0f, 0.0f, 0.0f, 1.0f };
	static const Rgba k_red = Rgba{ 1.0f, 0.0f, 0.0f, 1.0f };
	static const Rgba k_maroon = Rgba{ 0.5f, 0.0f, 0.0f, 1.0f };
	static const Rgba k_green = Rgba{ 0.0f, 1.0f, 0.0f, 1.0f };
	static const Rgba k_forest = Rgba{ 0.0f, 0.5f, 0.0f, 1.0f };
	static const Rgba k_blue = Rgba{ 0.0f, 0.0f, 1.0f, 1.0f };
	static const Rgba k_navy = Rgba{ 0.0f, 0.0f, 0.5f, 1.0f };
	static const Rgba k_blueprint = toRgba(0x14252e);
	static const Rgba k_yellow = Rgba{ 1.0f, 1.0f, 0.0f, 1.0f };
	static const Rgba k_olive = Rgba{ 0.5f, 0.5f, 0.0f, 1.0f };
	static const Rgba k_magenta = Rgba{ 1.0f, 0.0f, 1.0f, 1.0f };
	static const Rgba k_eggplant = Rgba{ 0.5f, 0.0f, 0.5f, 1.0f };
	static const Rgba k_cyan = Rgba{ 0.0f, 1.0f, 1.0f, 1.0f };
	static const Rgba k_teal = Rgba{ 0.0f, 0.5f, 0.5f, 1.0f };
	static const Rgba k_orange = Rgba{ 1.0f, 0.5f, 0.0f, 1.0f };
	static const Rgba k_rose = Rgba{ 1.0f, 0.0f, 0.5f, 1.0f };
	static const Rgba k_chartreuse = Rgba{ 0.5f, 1.0f, 0.0f, 1.0f };
	static const Rgba k_spring = Rgba{ 0.0f, 1.0f, 0.5f, 1.0f };
	static const Rgba k_violet = Rgba{ 0.5f, 0.0f, 1.0f, 1.0f };
	static const Rgba k_azure = Rgba{ 0.0f, 0.5f, 1.0f, 1.0f };

	inline auto toRgb(glm::vec3 const& a_vector)
	{
		return Rgb{
			ColorChannel{a_vector.x},
			ColorChannel{a_vector.y},
			ColorChannel{a_vector.z} };
	}

	inline auto toRgba(glm::vec3 const& a_vector)
	{
		return Rgba{
			ColorChannel{a_vector.x},
			ColorChannel{a_vector.y},
			ColorChannel{a_vector.z},
			1.0f };
	}

	inline auto toRgb(glm::vec4 const& a_vector)
	{
		return Rgb{
			ColorChannel{a_vector.x},
			ColorChannel{a_vector.y},
			ColorChannel{a_vector.z} };
	}

	inline auto toRgba(glm::vec4 const& a_vector)
	{
		return Rgba{
			ColorChannel{a_vector.x},
			ColorChannel{a_vector.y},
			ColorChannel{a_vector.z},
			ColorChannel{ a_vector.w } };
	}
}
