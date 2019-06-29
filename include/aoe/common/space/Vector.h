#pragma once

#include <aoe/core/visitor/Utils.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace aoe
{
	namespace common
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

	namespace visitor
	{
		template <typename VisitorType, typename ValueType
			, glm::qualifier Qualifier>
			void makeVisit(VisitorType& a_visitor
				, glm::vec<2, ValueType, Qualifier>& a_vector)
		{
			a_visitor.visit("X", a_vector.x);
			a_visitor.visit("Y", a_vector.y);
		}

		template <typename VisitorType, typename ValueType
			, glm::qualifier Qualifier>
			void makeVisit(VisitorType& a_visitor
				, glm::vec<3, ValueType, Qualifier>& a_vector)
		{
			a_visitor.visit("X", a_vector.x);
			a_visitor.visit("Y", a_vector.y);
			a_visitor.visit("Z", a_vector.z);
		}
	}
}
