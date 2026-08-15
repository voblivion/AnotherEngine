#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/data/ImageData.h>
#include <vob/aoe/rendering/resources/GpuTexture.h>
#include <vob/aoe/rendering/TextureSettings.h>


namespace vob::aoegl
{
	// TODO: maxAnisotropy should be a sampler parameter instead of a texture parameter
	GpuTexture createTexture(ImageData const& a_image, TextureSettings const& a_settings, float a_maxAnisotropy);
}
