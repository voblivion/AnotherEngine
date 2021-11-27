#pragma once

#include <glm/glm.hpp>

#include <vob/aoe/ecs/Component.h>


namespace vob::aoe::common
{
	struct LocalTransformComponent final
		: public aoecs::AComponent
	{
		// Attributes
		glm::mat4 m_matrix{ 1.0f };
	};
}

namespace vob::aoe::vis
{
	// TODO : const version
	template <typename VisitorType>
	void accept(VisitorType& a_visitor, common::LocalTransformComponent& a_this)
	{
		glm::vec3 position;
		a_visitor.visit(vis::makeNameValuePair("Position", position));
		glm::vec3 rotation;
		a_visitor.visit(vis::makeNameValuePair("Rotation", rotation));

		a_this.m_matrix = glm::translate(a_this.m_matrix, position);
		a_this.m_matrix *= glm::mat4{ glm::quat{ rotation } };
	}
}
