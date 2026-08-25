#pragma once

#include "vob/aoe/rendering/ShaderDefinition.h"
#include "vob/aoe/rendering/shaders/defines.h"
#include "vob/aoe/rendering/TextureDefinition.h"

#include "vob/misc/std/bounded_vector.h"

#include <glm/glm.hpp>

#include <memory>
#include <string>
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
}
