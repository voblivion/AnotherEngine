#pragma once

#include "vob/aoe/rendering/ShaderDefinition.h"
#include "vob/aoe/rendering/shaders/defines.h"
#include "vob/aoe/rendering/TextureDefinition.h"

#include "vob/misc/std/bounded_vector.h"

#include <memory>
#include <string>


namespace vob::aoegl
{
	struct MaterialDefinition
	{
		struct TextureEntry
		{
			std::string slotName;
			std::shared_ptr<TextureDefinition const> texture;
		};

		std::string name;
		std::shared_ptr<ShaderDefinition const> shader;

		mistd::bounded_vector<TextureEntry, k_materialTexturesCapacity> textures;

		// TODO: per-shader parameter block, codegen'd from parameters declared in
		// ShaderDefinition. All material variation is texture-driven for now.
	};
}
