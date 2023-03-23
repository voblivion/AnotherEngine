#pragma once

#include <vob/aoe/rendering/data/material_data.h>
#include <vob/aoe/rendering/data/mesh_data.h>

#include <memory>


namespace vob::aoegl
{
	struct textured_mesh_data
	{
		mesh_data m_mesh;
		material_data m_material;
	};

	struct model_data
	{
		std::pmr::vector<textured_mesh_data> m_texturedMeshes;
	};
}
