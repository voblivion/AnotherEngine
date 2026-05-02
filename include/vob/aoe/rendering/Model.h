#pragma once

#include <vob/aoe/rendering/GraphicTypes.h>

#include <cstdint>
#include <vector>


namespace vob::aoegl
{
	struct Mesh
	{
		GraphicId vao = k_invalidId;
		GraphicId vbo = k_invalidId;
		GraphicId ebo = k_invalidId;
		int32_t indexCount = 0;
	};

	struct Model
	{
		std::vector<Mesh> meshes;
		float boundingRadius = 0.0f;
	};
}
