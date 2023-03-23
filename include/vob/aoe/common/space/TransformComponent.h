#pragma once

#include <glm/glm.hpp>


namespace vob::aoe::common
{
	const auto g_right = glm::vec3{ 1.0f, 0.0f, 0.0f };
	const auto g_front = glm::vec3{ 0.0f, 1.0f, 0.0f };
	const auto g_up = glm::vec3{ 0.0f, 0.0f, 1.0f };

	struct TransformComponent final
	{
		// Attributes
		glm::mat4 m_matrix{ 1.0f };

	private:
	};

	float distance(TransformComponent const& a_lhs, TransformComponent const& a_rhs)
	{
		auto const& lhs = a_lhs.m_matrix[3];
		auto const& rhs = a_rhs.m_matrix[3];
		return glm::length((rhs - lhs));
	}
}

namespace vob::misvi
{
	template <typename VisitorType, typename ThisType>
	requires std::is_same_v<std::remove_cvref_t<ThisType>, aoe::common::TransformComponent>
	bool accept(VisitorType& a_visitor, ThisType& a_this)
	{
		glm::vec3 position;
		a_visitor.visit(misvi::nvp("Position", position));
		glm::vec3 rotation;
		a_visitor.visit(misvi::nvp("Rotation", rotation));

		a_this.m_matrix = glm::mat4{ 1.0f };
		a_this.m_matrix = glm::translate(a_this.m_matrix, position);
		a_this.m_matrix *= glm::mat4{ glm::quat{ rotation } };
		return true;
	}
}
