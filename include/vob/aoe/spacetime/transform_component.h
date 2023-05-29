#pragma once

#include <vob/misc/visitor/macros.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>


namespace vob::aoest
{
	struct transform_component
	{
		glm::mat4 m_matrix{ 1.0f };
	};
}

namespace vob::misvi
{
	VOB_MISVI_ACCEPT(aoest::transform_component)
	{
		glm::vec3 position{ 0.0f };
		a_visitor.visit(misvi::nvp("Position", position));
		glm::vec3 rotation{ 0.0f };
		a_visitor.visit(misvi::nvp("Rotation", rotation));

		a_value.m_matrix = glm::mat4{ 1.0f };
		a_value.m_matrix = glm::translate(a_value.m_matrix, position);
		a_value.m_matrix *= glm::mat4{ glm::quat{ rotation } };
		return true;
	}
}