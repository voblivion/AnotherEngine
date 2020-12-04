#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#ifdef _DEBUG
#define INIT_GRAPHIC_OBJECT = 0
#else
#define INIT_GRAPHIC_OBJECT 
#endif

namespace vob::aoe::common
{
	using GraphicObjectId = GLuint;
	using UniformLocation = GLint;

	inline auto getTranslation(glm::mat4 const& a_matrix)
	{
		return glm::vec3{ a_matrix[3] };
	}

	inline void setTranslation(glm::mat4& a_matrix, glm::vec3 const& a_position)
	{
		a_matrix[3] = glm::vec4{ a_position, 1.0f };
	}

	inline auto getRotation(glm::mat4 const& a_matrix)
	{
		return glm::quat{ a_matrix };
	}

	inline void setRotation(glm::mat4& a_matrix, glm::mat4 const& a_orientation)
	{
		a_matrix[0] = a_orientation[0];
		a_matrix[1] = a_orientation[1];
		a_matrix[2] = a_orientation[2];
	}

	inline void setRotation(glm::mat4& a_matrix, glm::quat const& a_orientation)
	{
		setRotation(a_matrix, glm::mat4_cast(a_orientation));
	}

	inline auto translation(glm::vec3 const& a_position)
	{
		return glm::translate(glm::mat4{ 1.0f }, a_position);
	}
}
