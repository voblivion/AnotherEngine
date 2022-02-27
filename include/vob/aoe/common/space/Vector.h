#pragma once

#include <glm/geometric.hpp>

#include <vob/misc/visitor/name_value_pair.h>


namespace vob::aoe::common
{
	template <glm::length_t t_length, typename Type, glm::qualifier t_qualifier>
	inline float squaredLength(glm::vec<t_length, Type, t_qualifier> const& a_vector)
	{
		return glm::dot(a_vector, a_vector);
	}

	template <glm::length_t t_length, typename Type, glm::qualifier t_qualifier>
	inline bool isNullWithEpsilon(
		glm::vec<t_length, Type, t_qualifier> const& a_vector
		, float const a_epsilon = FLT_EPSILON
	)
	{
		return squaredLength(a_vector) < a_epsilon * a_epsilon;
	}
}

namespace vob::misvi
{
	template <glm::length_t t_component
		, typename VisitorType, glm::length_t t_length, typename Type, glm::qualifier t_qualifier>
	std::enable_if_t<t_component >= t_length> tryVisitVectorComponent(
		VisitorType& a_visitor
		, glm::vec<t_length, Type, t_qualifier>& a_vector
		, std::string_view const a_componentName
	) {}

	template <glm::length_t t_component
		, typename VisitorType, glm::length_t t_length, typename Type, glm::qualifier t_qualifier>
	std::enable_if_t<t_component < t_length> tryVisitVectorComponent(
		VisitorType& a_visitor
		, glm::vec<t_length, Type, t_qualifier>& a_vector
		, std::string_view const a_componentName
	)
	{
		a_visitor.visit(misvi::nvp(a_componentName, a_vector[t_component]));
	}

	template <typename VisitorType, glm::length_t t_length, typename Type, glm::qualifier t_qualifier>
	bool accept(
		VisitorType& a_visitor
		, glm::vec<t_length, Type, t_qualifier>& a_vector
	)
	{
		tryVisitVectorComponent<0>(a_visitor, a_vector, "X");
		tryVisitVectorComponent<1>(a_visitor, a_vector, "Y");
		tryVisitVectorComponent<2>(a_visitor, a_vector, "Z");
		tryVisitVectorComponent<3>(a_visitor, a_vector, "W");
		return true;
	}

	template <glm::length_t t_component
		, typename VisitorType, glm::length_t t_length, typename Type, glm::qualifier t_qualifier>
	std::enable_if_t<t_component >= t_length> tryVisitVectorComponent(
		VisitorType& a_visitor
		, glm::vec<t_length, Type, t_qualifier> const& a_vector
		, std::string_view const a_componentName
	) {}

	template <glm::length_t t_component
		, typename VisitorType, glm::length_t t_length, typename Type, glm::qualifier t_qualifier>
		std::enable_if_t < t_component < t_length> tryVisitVectorComponent(
			VisitorType& a_visitor
			, glm::vec<t_length, Type, t_qualifier> const& a_vector
			, std::string_view const a_componentName
		)
	{
		a_visitor.visit(misvi::nvp(a_componentName, a_vector[t_component]));
	}

	template <typename VisitorType, glm::length_t t_length, typename Type, glm::qualifier t_qualifier>
	bool accept(
		VisitorType& a_visitor
		, glm::vec<t_length, Type, t_qualifier> const& a_vector
	)
	{
		tryVisitVectorComponent<0>(a_visitor, a_vector, "X");
		tryVisitVectorComponent<1>(a_visitor, a_vector, "Y");
		tryVisitVectorComponent<2>(a_visitor, a_vector, "Z");
		tryVisitVectorComponent<3>(a_visitor, a_vector, "W");
		return true;
	}
}
