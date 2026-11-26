#pragma once

#include "vob/aoe/rendering/data/ImageData.h"
#include "vob/aoe/rendering/TextureSettings.h"

#include <vob/aoe/data/SoftPtr.h>

#include <vob/misc/visitor/name_value_pair.h>

#include <type_traits>


namespace vob::aoegl
{
	struct TextureDefinition
	{
		aoedt::SoftPtr<ImageData> image;
		TextureSettings settings;
	};

	template <typename TVisitor, typename TDefinition>
		requires std::is_same_v<std::remove_cvref_t<TDefinition>, TextureDefinition>
	bool accept(TVisitor& a_visitor, TDefinition& a_definition)
	{
		auto result = true;
		result &= a_visitor.visit(misvi::nvp("image", a_definition.image));
		result &= a_visitor.visit(misvi::nvp("settings", a_definition.settings));
		return result;
	}
}
