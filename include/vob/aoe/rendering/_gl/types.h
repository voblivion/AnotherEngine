#pragma once

#include <GL/glew.h>

#include <array>


namespace vob::aoegl
{
	using graphic_id = GLuint;
	using graphic_int = GLint;
	using graphic_size = GLsizei;
	using graphic_uniform_location = GLint;
	using graphic_enum = GLenum;
	using triangle_data = std::array<graphic_id, 3>;
}