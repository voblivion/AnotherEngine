#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace vob::aoegl::view_util
{
	glm::mat4 create_mesh_normal_transform(glm::mat4 const& a_meshTransform)
	{
		return glm::transpose(glm::inverse(a_meshTransform));
	}

	glm::mat4 create_view_projection_matrix(
		glm::mat4 const& a_cameraTransform
		, float a_fovDegree
		, float a_windowWidth
		, float a_windowHeight
		, float a_nearClip
		, float a_farClip)
	{
		return glm::perspective(
			glm::radians(a_fovDegree)
			, a_windowWidth / a_windowHeight
			, a_nearClip
			, a_farClip) * glm::inverse(a_cameraTransform);
	}
}
