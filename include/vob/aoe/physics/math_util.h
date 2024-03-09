#pragma once

#include <bullet/LinearMath/btVector3.h>
#include <bullet/LinearMath/btTransform.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace vob::aoeph
{
	inline btVector3 to_bt(glm::vec3 const& a_glmVector3)
	{
		return btVector3{ a_glmVector3.x, a_glmVector3.y, a_glmVector3.z };
	}

	inline btTransform to_bt(glm::mat4 const& a_glmTransform)
	{
		btTransform btTransformMatrix;
		btTransformMatrix.setFromOpenGLMatrix(glm::value_ptr(a_glmTransform));
		return btTransformMatrix;
	}

	inline glm::vec3 to_glm(btVector3 const& a_btVector)
	{
		return glm::vec3{ a_btVector[0], a_btVector[1], a_btVector[2] };
	}

	inline glm::mat4 to_glm(btTransform const& a_btTransform)
	{
		glm::mat4 glmTransform;
		a_btTransform.getOpenGLMatrix(glm::value_ptr(glmTransform));
		return glmTransform;
	}
}
