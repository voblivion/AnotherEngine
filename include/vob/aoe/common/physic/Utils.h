#pragma once

#include <LinearMath/btVector3.h>
#include <vob/aoe/common/space/Vector.h>
#include <vob/aoe/common/space/Quaternion.h>

namespace vob::aoe::common
{
	inline btVector3 toBtVector(Vector3 const& a_vec)
	{
		return { a_vec.x, a_vec.y, a_vec.z };
	}

	inline Vector3 toGlmVector(btVector3 const& a_vec)
	{
		return { a_vec.x(), a_vec.y(), a_vec.z() };
	}

	inline btQuaternion toBtQuaternion(Quaternion const& a_qua)
	{
		return { a_qua.x, a_qua.y, a_qua.z, a_qua.w };
	}

	inline Quaternion toGlmQuaternion(btQuaternion const& a_qua)
	{
		return { a_qua.w(), a_qua.x(), a_qua.y(), a_qua.z() };
	}

	inline glm::vec3 toGlmVec3(btVector3 const& a_vector)
	{
		return glm::vec3{ a_vector.x(), a_vector.y(), a_vector.z() };
	}
}
