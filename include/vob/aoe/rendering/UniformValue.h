#pragma once

#include "vob/misc/std/string_vector_map.h"

#include <glm/glm.hpp>

#include <array>
#include <cstdint>
#include <string_view>
#include <variant>


namespace vob::aoegl
{
	using UniformValue = std::variant<int32_t, float, glm::vec2, glm::vec3, glm::vec4>;

	using UniformValueMap = mistd::string_vector_map<UniformValue>;

	namespace detail
	{
		constexpr auto k_uniformSizes = std::array<uint32_t, 5>{ 4, 4, 8, 12, 16 };
		constexpr auto k_uniformAlignments = std::array<uint32_t, 5>{ 4, 4, 8, 16, 16 };
		constexpr auto k_uniformGlslTypes = std::array<std::string_view, 5>{ "int", "float", "vec2", "vec3", "vec4" };
	}

	constexpr uint32_t getUniformSize(size_t a_variantIndex)
	{
		return detail::k_uniformSizes[a_variantIndex];
	}

	constexpr uint32_t getUniformAlignment(size_t a_variantIndex)
	{
		return detail::k_uniformAlignments[a_variantIndex];
	}

	constexpr std::string_view getUniformGlslType(size_t a_variantIndex)
	{
		return detail::k_uniformGlslTypes[a_variantIndex];
	}

	inline UniformValue makeUniformValue(size_t a_variantIndex)
	{
		switch (a_variantIndex)
		{
		case 1: return float{};
		case 2: return glm::vec2{};
		case 3: return glm::vec3{};
		case 4: return glm::vec4{};
		default: return int32_t{};
		}
	}
}
