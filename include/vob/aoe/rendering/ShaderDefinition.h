#pragma once

#include "vob/aoe/rendering/data/ShaderSource.h"
#include "vob/aoe/rendering/GraphicTypes.h"
#include "vob/aoe/rendering/shaders/defines.h"
#include "vob/aoe/rendering/ShadingPass.h"
#include "vob/aoe/rendering/TextureSettings.h"
#include "vob/aoe/rendering/UniformValue.h"

#include "vob/misc/std/bounded_vector.h"

#include <vob/aoe/data/SoftPtr.h>

#include <vob/misc/visitor/accept.h>
#include <vob/misc/visitor/name_value_pair.h>

#include <glm/glm.hpp>

#include <string>
#include <type_traits>
#include <vector>


namespace vob::aoegl
{
	struct ShaderDefinition
	{
		struct TextureSlot
		{
			std::string name;
			TextureSettings::SamplerType samplerType = TextureSettings::SamplerType::Simple;
			glm::vec4 fallbackColor = glm::vec4{ 1.0f };
		};

		std::string name;
		ShadingPass shadingPass = ShadingPass::Opaque;
		aoedt::SoftPtr<ShaderSource> partialSource;
		std::vector<std::string> defines;

		mistd::bounded_vector<TextureSlot, k_materialTexturesCapacity> textureSlots;
		mistd::bounded_vector<int32_t, k_materialTexturesCapacity> depthOnlyTextureSlotIndices;

		UniformValueMap uniformDefaults;

		bool isAlphaMasked = false;
	};

	template <typename TVisitor, typename TSlot>
		requires std::is_same_v<std::remove_cvref_t<TSlot>, ShaderDefinition::TextureSlot>
	bool accept(TVisitor& a_visitor, TSlot& a_slot)
	{
		auto result = true;
		result &= a_visitor.visit(misvi::nvp("name", a_slot.name));
		result &= a_visitor.visit(misvi::nvp("samplerType", a_slot.samplerType));
		result &= a_visitor.visit(misvi::nvp("fallbackColor", a_slot.fallbackColor));
		return result;
	}

	template <typename TVisitor, typename TDefinition>
		requires std::is_same_v<std::remove_cvref_t<TDefinition>, ShaderDefinition>
	bool accept(TVisitor& a_visitor, TDefinition& a_definition)
	{
		auto uniformDefaults = UniformValueMapRef{ a_definition.uniformDefaults };

		auto result = true;
		result &= a_visitor.visit(misvi::nvp("name", a_definition.name));
		result &= a_visitor.visit(misvi::nvp("shadingPass", a_definition.shadingPass));
		result &= a_visitor.visit(misvi::nvp("partialSource", a_definition.partialSource));
		result &= a_visitor.visit(misvi::nvp("defines", a_definition.defines));
		result &= a_visitor.visit(misvi::nvp("textureSlots", a_definition.textureSlots));
		result &= a_visitor.visit(misvi::nvp("depthOnlyTextureSlotIndices", a_definition.depthOnlyTextureSlotIndices));
		result &= a_visitor.visit(misvi::nvp("uniformDefaults", uniformDefaults));
		result &= a_visitor.visit(misvi::nvp("isAlphaMasked", a_definition.isAlphaMasked));
		return result;
	}
}
