#pragma once

#include <vob/aoe/rendering/resources/program.h>


namespace vob::aoegl
{
	struct debug_render_world_component
	{
		graphic_id m_vao;
		graphic_id m_vbo;
		graphic_id m_ebo;
		debug_program m_debugProgram;
	};
}
