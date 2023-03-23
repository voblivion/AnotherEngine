#pragma once

#include <vob/aoe/rendering/graphic_types.h>
#include <vob/aoe/rendering/resources/program.h>
#include <vob/aoe/rendering/resources/material.h>
#include <vob/aoe/rendering/resources/mesh.h>
#include <vob/aoe/rendering/resources/textured_mesh.h>

#include <span>


namespace vob::aoegl
{
	struct program
	{
		graphic_id m_id = 0;
	};

	struct scene_program : public program
	{
		graphic_uniform_location m_viewPositionLocation = GL_INVALID_VALUE;
		graphic_uniform_location m_viewProjectionTransformLocation = GL_INVALID_VALUE;
	};

	struct debug_program : public scene_program
	{
	};

	struct mesh_program : public scene_program
	{
		static constexpr std::size_t k_maxRigSize = 64u;

		graphic_uniform_location m_ambientColorLocation = GL_INVALID_VALUE;
		graphic_uniform_location m_sunColorLocation = GL_INVALID_VALUE;
		graphic_uniform_location m_sunDirectionLocation = GL_INVALID_VALUE;

		graphic_uniform_location m_meshTransformLocation = GL_INVALID_VALUE;
		graphic_uniform_location m_meshNormalTransformLocation = GL_INVALID_VALUE;
		graphic_uniform_location m_isRiggedLocation = GL_INVALID_VALUE;
		graphic_uniform_location m_rigPoseLocation = GL_INVALID_VALUE;
	};

	struct post_process_program : public program
	{
		graphic_uniform_location m_windowSize = GL_INVALID_VALUE;
	};
}
