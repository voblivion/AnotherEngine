#pragma once

#include <vob/aoe/rendering/RenderTexture.h>


namespace vob::aoegl
{
	struct SceneTextureContext
	{
		RenderTexture texture;

		float near = 0.0f;
		float far = 0.0f;
	};
}
