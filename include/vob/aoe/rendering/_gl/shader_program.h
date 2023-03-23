#pragma once

#include "types.h"

namespace vob::aoegl
{
	struct shader_program
	{
		graphic_id m_id;
	};

	struct scene_shader_program : public shader_program
	{
		graphic_uniform_location m_viewPositionLocation;
		graphic_uniform_location m_viewProjectionTransformLocation;
	};

	struct mesh_shader_program : public scene_shader_program
	{
		graphic_uniform_location m_ambientColorLocation;
		graphic_uniform_location m_sunColorLocation;
		graphic_uniform_location m_sunDirectionLocation;

		graphic_uniform_location m_meshTransformLocation;
		graphic_uniform_location m_meshNormalTransformLocation;
		graphic_uniform_location m_rigPoseLocation;
	};
}
