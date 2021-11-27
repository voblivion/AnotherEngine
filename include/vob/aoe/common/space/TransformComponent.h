#pragma once

#include <vob/aoe/ecs/Component.h>


namespace vob::aoe::common
{
	const auto g_right = glm::vec3{ 1.0f, 0.0f, 0.0f };
	const auto g_front = glm::vec3{ 0.0f, 1.0f, 0.0f };
	const auto g_up = glm::vec3{ 0.0f, 0.0f, 1.0f };

	struct TransformComponent final
		: public aoecs::AComponent
	{
		// Attributes
		glm::mat4 m_matrix{ 1.0f };

	private:
	};
}

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::TransformComponent, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
		glm::vec3 position;
		a_visitor.visit(vis::makeNameValuePair("Position", position));
		glm::vec3 rotation;
		a_visitor.visit(vis::makeNameValuePair("Rotation", rotation));

		a_this.m_matrix = glm::mat4{ 1.0f };
		a_this.m_matrix = glm::translate(a_this.m_matrix, position);
		a_this.m_matrix *= glm::mat4{ glm::quat{ rotation } };
	}
}
