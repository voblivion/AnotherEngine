#pragma once

#include <vob/aoe/rendering/resources/program.h>


namespace vob::aoegl
{
	struct mesh_render_world_component
	{
		graphic_id m_vao;
		mesh_program m_meshProgram;
	};
}
