#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"
#include "vob/aoe/rendering/shaders/defines.h"
#include "vob/aoe/rendering/ShadingPass.h"
#include "vob/aoe/rendering/TextureSettings.h"

#include "vob/misc/std/bounded_vector.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <string>
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
		std::filesystem::path partialSourcePath;
		std::vector<std::string> defines;

		mistd::bounded_vector<TextureSlot, k_materialTexturesCapacity> textureSlots;
		mistd::bounded_vector<int32_t, k_materialTexturesCapacity> depthOnlyTextureSlotIndices;

		bool hasOpacitySupport = false;
		bool isTwoSided = false;
	};
}
