#pragma once

#include <vob/aoe/core/visitor/Utils.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace vob::aoe::common
{
	using Vector2 = glm::vec2;
	using Vector3 = glm::vec3;

	inline float squaredLength(Vector2 const& a_vector)
	{
		return a_vector.x * a_vector.x + a_vector.y * a_vector.y;
	}

	inline float squaredLength(Vector3 const& a_vector)
	{
		return a_vector.x * a_vector.x + a_vector.y * a_vector.y
			+ a_vector.z * a_vector.z;
	}

	inline bool isNullWithEpsilon(Vector2 const& a_vector
		, float const a_epsilon = FLT_EPSILON)
	{
		return squaredLength(a_vector) < a_epsilon * a_epsilon;
	}

	inline bool isNullWithEpsilon(Vector3 const& a_vector
		, float const a_epsilon = FLT_EPSILON)
	{
		return squaredLength(a_vector) < a_epsilon * a_epsilon;
	}
}

namespace vob::aoe::vis
{
	template <typename VisitorType>
	void accept(VisitorType& a_visitor, aoe::common::Vector2& a_vector)
	{
		a_visitor.visit(vis::makeNameValuePair("X", a_vector.x));
		a_visitor.visit(vis::makeNameValuePair("Y", a_vector.y));
	}

	template <typename VisitorType>
	void accept(VisitorType& a_visitor, aoe::common::Vector2 const& a_vector)
	{
		a_visitor.visit(vis::makeNameValuePair("X", a_vector.x));
		a_visitor.visit(vis::makeNameValuePair("Y", a_vector.y));
	}

	template <typename VisitorType>
	void accept(VisitorType& a_visitor, aoe::common::Vector3& a_vector)
	{
		a_visitor.visit(vis::makeNameValuePair("X", a_vector.x));
		a_visitor.visit(vis::makeNameValuePair("Y", a_vector.y));
		a_visitor.visit(vis::makeNameValuePair("Z", a_vector.z));
	}

	template <typename VisitorType>
	void accept(VisitorType& a_visitor, aoe::common::Vector3 const& a_vector)
	{
		a_visitor.visit(vis::makeNameValuePair("X", a_vector.x));
		a_visitor.visit(vis::makeNameValuePair("Y", a_vector.y));
		a_visitor.visit(vis::makeNameValuePair("Z", a_vector.z));
	}
}
