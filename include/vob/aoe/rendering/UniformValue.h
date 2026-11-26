#pragma once

#include "vob/misc/std/string_vector_map.h"

#include <vob/aoe/data/GlmAccept.h>

#include <vob/misc/visitor/index_value_pair.h>
#include <vob/misc/visitor/name_value_pair.h>
#include <vob/misc/visitor/size_tag.h>

#include <glm/glm.hpp>

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
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

	template <typename TUniformValue>
	struct UniformValueRef
	{
		TUniformValue& value;
	};

	template <typename TVisitor, typename TUniformValue>
	bool accept(TVisitor& a_visitor, UniformValueRef<TUniformValue>& a_ref)
	{
		if constexpr (std::is_const_v<TUniformValue>)
		{
			return std::visit(
				[&a_visitor, &a_ref](auto const& a_value)
				{
					return a_visitor.visit(misvi::nvp(getUniformGlslType(a_ref.value.index()), a_value));
				},
				a_ref.value);
		}
		else
		{
			for (auto index = size_t{ 0 }; index < std::variant_size_v<UniformValue>; ++index)
			{
				auto value = makeUniformValue(index);
				auto const isRead = std::visit(
					[&a_visitor, index](auto& a_value)
					{
						return a_visitor.visit(misvi::nvp(getUniformGlslType(index), a_value));
					},
					value);
				if (isRead)
				{
					a_ref.value = std::move(value);
					return true;
				}
			}
			return false;
		}
	}

	template <typename TUniformValue>
	struct UniformValueEntryRef
	{
		std::string name;
		UniformValueRef<TUniformValue> value;
	};

	template <typename TVisitor, typename TUniformValue>
	bool accept(TVisitor& a_visitor, UniformValueEntryRef<TUniformValue>& a_ref)
	{
		auto result = true;
		result &= a_visitor.visit(misvi::nvp("name", a_ref.name));
		result &= a_visitor.visit(misvi::nvp("value", a_ref.value));
		return result;
	}

	template <typename TUniformValueMap>
	struct UniformValueMapRef
	{
		TUniformValueMap& map;
	};

	template <typename TVisitor, typename TUniformValueMap>
	bool accept(TVisitor& a_visitor, UniformValueMapRef<TUniformValueMap>& a_ref)
	{
		if constexpr (std::is_const_v<TUniformValueMap>)
		{
			auto sizeTag = misvi::size_tag{ a_ref.map.size() };
			a_visitor.visit(sizeTag);
			auto index = size_t{ 0 };
			for (auto const& [key, value] : a_ref.map)
			{
				auto entry = UniformValueEntryRef<UniformValue const>{ std::string{ key.view() }, { value } };
				a_visitor.visit(misvi::ivp(index++, entry));
			}
			return true;
		}
		else
		{
			auto sizeTag = misvi::size_tag{};
			if (!a_visitor.visit(sizeTag))
			{
				return false;
			}

			a_ref.map.clear();
			for (auto index = size_t{ 0 }; index < sizeTag.size; ++index)
			{
				auto value = UniformValue{};
				auto entry = UniformValueEntryRef<UniformValue>{ {}, { value } };
				if (a_visitor.visit(misvi::ivp(index, entry)))
				{
					a_ref.map.emplace(std::move(entry.name), std::move(value));
				}
			}
			return true;
		}
	}
}
