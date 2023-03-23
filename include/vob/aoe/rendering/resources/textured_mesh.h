#pragma once

#include <vob/aoe/rendering/resources/material.h>
#include <vob/aoe/rendering/resources/mesh.h>

namespace vob::aoegl
{
	struct textured_mesh
	{
		mesh m_mesh;
		material m_material;
	};
}
