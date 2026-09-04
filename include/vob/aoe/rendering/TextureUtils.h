#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/data/ImageData.h>
#include <vob/aoe/rendering/resources/GpuResource.h>
#include <vob/aoe/rendering/TextureSettings.h>

#include <glm/glm.hpp>


namespace vob::aoegl
{
	// TODO: maxAnisotropy should be a sampler parameter instead of a texture parameter
	GpuTexture createTexture(
		GpuDeleteQueue& a_deleteQueue
		, ImageData const& a_image
		, TextureSettings const& a_settings
		, float a_maxAnisotropy);

	GpuTexture createSolidColorTexture(GpuDeleteQueue& a_deleteQueue, glm::vec4 const& a_color);
}
