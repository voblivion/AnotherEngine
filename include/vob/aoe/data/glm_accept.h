#pragma once

#include <glm/glm.hpp>

#include <vob/misc/visitor/name_value_pair.h>


namespace vob::misvi
{
	namespace detail
	{
		template <
			glm::length_t t_component,
			typename TVisitor,
			glm::length_t t_length,
			typename TValue,
			glm::qualifier t_qualifier>
		std::enable_if_t<t_component >= t_length> try_visit_component(
			TVisitor& a_visitor
			, glm::vec<t_length, TValue, t_qualifier>& a_vector
			, std::string_view const a_componentName
		)
		{}

		template <
			glm::length_t t_component,
			typename TVisitor,
			glm::length_t t_length,
			typename TValue,
			glm::qualifier t_qualifier>
		std::enable_if_t< t_component < t_length> try_visit_component(
			TVisitor& a_visitor
			, glm::vec<t_length, TValue, t_qualifier>& a_vector
			, std::string_view const a_componentName)
		{
			a_visitor.visit(misvi::nvp(a_componentName, a_vector[t_component]));
		}

		template <
			glm::length_t t_component,
			typename TVisitor,
			glm::length_t t_length,
			typename TValue,
			glm::qualifier t_qualifier>
		std::enable_if_t<t_component >= t_length> try_visit_component(
			TVisitor& a_visitor
			, glm::vec<t_length, TValue, t_qualifier> const& a_vector
			, std::string_view const a_componentName
		)
		{}

		template <
			glm::length_t t_component,
			typename TVisitor,
			glm::length_t t_length,
			typename TValue,
			glm::qualifier t_qualifier>
		std::enable_if_t < t_component < t_length> try_visit_component(
			TVisitor& a_visitor
			, glm::vec<t_length, TValue, t_qualifier> const& a_vector
			, std::string_view const a_componentName)
		{
			a_visitor.visit(misvi::nvp(a_componentName, a_vector[t_component]));
		}
	}

	template <typename TVisitor, glm::length_t t_length, typename TValue, glm::qualifier t_qualifier>
	bool accept(TVisitor& a_visitor, glm::vec<t_length, TValue, t_qualifier>& a_vector)
	{
		detail::try_visit_component<0>(a_visitor, a_vector, "X");
		detail::try_visit_component<1>(a_visitor, a_vector, "Y");
		detail::try_visit_component<2>(a_visitor, a_vector, "Z");
		detail::try_visit_component<3>(a_visitor, a_vector, "W");
		return true;
	}

	template <typename TVisitor, glm::length_t t_length, typename TValue, glm::qualifier t_qualifier>
	bool accept(TVisitor& a_visitor, glm::vec<t_length, TValue, t_qualifier> const& a_vector)
	{
		detail::try_visit_component<0>(a_visitor, a_vector, "X");
		detail::try_visit_component<1>(a_visitor, a_vector, "Y");
		detail::try_visit_component<2>(a_visitor, a_vector, "Z");
		detail::try_visit_component<3>(a_visitor, a_vector, "W");
		return true;
	}
}
