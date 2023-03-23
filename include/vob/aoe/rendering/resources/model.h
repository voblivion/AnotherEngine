#pragma once

#include <vob/aoe/rendering/resources/textured_mesh.h>

#include <vector>


namespace vob::aoegl
{
	struct model
	{
		std::pmr::vector<textured_mesh> m_texturedMeshes;
	};
}
