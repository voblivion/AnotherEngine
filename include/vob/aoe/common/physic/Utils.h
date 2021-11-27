#pragma once

#include <LinearMath/btVector3.h>

#include <glm/gtc/type_ptr.hpp>

#include <vob/aoe/common/space/Vector.h>
#include <vob/aoe/common/space/Quaternion.h>

namespace vob::aoe::common
{
	inline btVector3 toBtVector(glm::vec3 const& a_vec)
	{
		return { a_vec.x, a_vec.y, a_vec.z };
	}

	inline btQuaternion toBtQuaternion(glm::quat const& a_qua)
	{
		return { a_qua.x, a_qua.y, a_qua.z, a_qua.w };
	}

	inline btTransform toBtTransform(glm::mat4 const& a_matrix)
	{
		btTransform transform{};
		transform.setFromOpenGLMatrix(glm::value_ptr(a_matrix));
		return transform;
	}

	inline glm::quat toGlmQuaternion(btQuaternion const& a_qua)
	{
		return { a_qua.w(), a_qua.x(), a_qua.y(), a_qua.z() };
	}

	inline glm::vec3 toGlmVec3(btVector3 const& a_vector)
	{
		return glm::vec3{ a_vector.x(), a_vector.y(), a_vector.z() };
	}

	inline glm::mat4 toGlmMat4(btTransform const& a_transform)
	{
		glm::mat4 matrix;
		a_transform.getOpenGLMatrix(glm::value_ptr(matrix));
		return matrix;
	}
}
