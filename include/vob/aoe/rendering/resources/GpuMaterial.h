#pragma once

#include "vob/aoe/rendering/resources/GpuShader.h"
#include "vob/aoe/rendering/resources/GpuTexture.h"
#include "vob/aoe/rendering/resources/Handle.h"

#include "vob/misc/std/bounded_vector.h"
#include "vob/misc/std/container_util.h"

#include "vob/aoe/rendering/shaders/defines.h"

#include <memory>


namespace vob::aoegl
{
	struct GpuMaterial
	{
		struct TextureSlot
		{
			Handle<GpuTexture> texture;
			GraphicId id = k_invalidId;
		};

		Handle<GpuShader> shader;
		GraphicId paramsUbo;
		mistd::bounded_vector<TextureSlot, k_materialTexturesCapacity> textures;
		mistd::bounded_vector<int32_t, k_materialTexturesCapacity> depthOnlyTextureSlotIndices;
		bool isTwoSided;
	};
}
