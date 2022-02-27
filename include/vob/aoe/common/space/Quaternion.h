#pragma once

#include <vob/aoe/common/data/filesystem/FileSystemIndexer.h>

#include <glm/ext/quaternion_float.hpp>
#include <glm/gtc/quaternion.hpp>

namespace vob::aoe::common
{
	inline bool isNullWithEpsilon(glm::quat const& a_quaternion
		, float const a_epsilon = FLT_EPSILON)
	{
		return a_quaternion.x * a_quaternion.x + a_quaternion.y * a_quaternion.y
			+ a_quaternion.z * a_quaternion.z < FLT_EPSILON * FLT_EPSILON;
	}
}

namespace vob::misvi
{
	template <typename VisitorType>
	bool accept(VisitorType& a_visitor, glm::quat& a_quaternion)
	{
		glm::vec3 t_eulerAngles{};
		a_visitor.visit(t_eulerAngles);
		a_quaternion = glm::quat{ t_eulerAngles };
		return true;
	}

	template <typename VisitorType>
	bool accept(VisitorType& a_visitor, glm::quat const& a_quaternion)
	{
		auto const t_eulerAngles = glm::eulerAngles(a_quaternion);
		a_visitor.visit(t_eulerAngles);
		return true;
	}
}
