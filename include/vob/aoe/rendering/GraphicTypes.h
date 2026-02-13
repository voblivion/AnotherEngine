#pragma once

#include <GL/glew.h>

#include <array>

namespace vob::aoegl
{
	using GraphicId = GLuint;
	using GraphicIndex = GLuint;
	using GraphicSize = GLsizei;
	using GraphicUniformLocation = GLint;
	using GraphicEnum = GLenum;
	using GraphicInt = GLint;

	static constexpr GraphicId k_invalidId = 0;
	static constexpr GraphicUniformLocation k_invalidUniformLocation = -1;
}
