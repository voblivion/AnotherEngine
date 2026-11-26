#pragma once

#include "vob/aoe/rendering/ShaderDefinition.h"
#include "vob/aoe/rendering/shaders/defines.h"
#include "vob/aoe/rendering/TextureDefinition.h"
#include "vob/aoe/rendering/UniformValue.h"

#include "vob/misc/std/bounded_vector.h"

#include <vob/misc/visitor/name_value_pair.h>

#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <type_traits>
#include <variant>


namespace vob::aoegl
{
	struct MaterialDefinition
	{
		struct TextureEntry
		{
			std::string slotName;
			std::variant<std::shared_ptr<TextureDefinition const>, glm::vec4> source;
		};

		std::string name;
		std::shared_ptr<ShaderDefinition const> shader;

		mistd::bounded_vector<TextureEntry, k_materialTexturesCapacity> textures;

		UniformValueMap uniformValues;

		bool isTwoSided = false;
	};

	template <typename TVisitor, typename TEntry>
		requires std::is_same_v<std::remove_cvref_t<TEntry>, MaterialDefinition::TextureEntry>
	bool accept(TVisitor& a_visitor, TEntry& a_entry)
	{
		auto result = a_visitor.visit(misvi::nvp("slotName", a_entry.slotName));
		if constexpr (std::is_const_v<TEntry>)
		{
			if (auto const* const texture = std::get_if<std::shared_ptr<TextureDefinition const>>(&a_entry.source))
			{
				result &= a_visitor.visit(misvi::nvp("texture", *texture));
			}
			else
			{
				result &= a_visitor.visit(misvi::nvp("color", std::get<glm::vec4>(a_entry.source)));
			}
		}
		else
		{
			auto texture = std::shared_ptr<TextureDefinition const>{};
			auto color = glm::vec4{};
			if (a_visitor.visit(misvi::nvp("texture", texture)))
			{
				a_entry.source = std::move(texture);
			}
			else if (a_visitor.visit(misvi::nvp("color", color)))
			{
				a_entry.source = color;
			}
			else
			{
				result = false;
			}
		}
		return result;
	}

	template <typename TVisitor, typename TDefinition>
		requires std::is_same_v<std::remove_cvref_t<TDefinition>, MaterialDefinition>
	bool accept(TVisitor& a_visitor, TDefinition& a_definition)
	{
		auto uniformValues = UniformValueMapRef{ a_definition.uniformValues };

		auto result = true;
		result &= a_visitor.visit(misvi::nvp("name", a_definition.name));
		result &= a_visitor.visit(misvi::nvp("shader", a_definition.shader));
		result &= a_visitor.visit(misvi::nvp("textures", a_definition.textures));
		result &= a_visitor.visit(misvi::nvp("uniformValues", uniformValues));
		result &= a_visitor.visit(misvi::nvp("isTwoSided", a_definition.isTwoSided));
		return result;
	}
}
