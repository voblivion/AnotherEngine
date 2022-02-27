#pragma once

#include <vob/aoe/common/data/filesystem/FileSystemIndexer.h>

#include <glm/glm.hpp>


namespace vob::aoe::common
{
	struct LocalTransformComponent final
	{
		// Attributes
		glm::mat4 m_matrix{ 1.0f };
	};
}

namespace vob::misvi
{
	// TODO : const version
	template <typename VisitorType>
	bool accept(VisitorType& a_visitor, aoe::common::LocalTransformComponent& a_this)
	{
		glm::vec3 position;
		a_visitor.visit(misvi::nvp("Position", position));
		glm::vec3 rotation;
		a_visitor.visit(misvi::nvp("Rotation", rotation));

		a_this.m_matrix = glm::translate(a_this.m_matrix, position);
		a_this.m_matrix *= glm::mat4{ glm::quat{ rotation } };
		return true;
	}
}
