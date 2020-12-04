#pragma once

#include <vob/aoe/core/type/Primitive.h>
#include <vob/aoe/core/visitor/Utils.h>
#include <glm/ext/quaternion_float.hpp>
#include <glm/gtc/quaternion.hpp>

namespace vob::aoe::common
{
	inline bool isNullWithEpsilon(quat const& a_quaternion
		, float const a_epsilon = FLT_EPSILON)
	{
		return a_quaternion.x * a_quaternion.x + a_quaternion.y * a_quaternion.y
			+ a_quaternion.z * a_quaternion.z < FLT_EPSILON * FLT_EPSILON;
	}
}

namespace vob::aoe::vis
{
	template <typename VisitorType>
	void accept(VisitorType& a_visitor, quat& a_quaternion)
	{
		vec3 t_eulerAngles{};
		a_visitor.visit(t_eulerAngles);
		a_quaternion = quat{ t_eulerAngles };
	}

	template <typename VisitorType>
	void accept(VisitorType& a_visitor, quat const& a_quaternion)
	{
		auto const t_eulerAngles = glm::eulerAngles(a_quaternion);
		a_visitor.visit(t_eulerAngles);
	}
}
