#pragma once

#include <vob/aoe/data/glm_accept.h>

#include <vob/misc/std/message_macros.h>
#include <vob/misc/visitor/macros.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>


namespace vob::aoest
{
	struct transform_component
	{
#pragma message(VOB_MISTD_TODO "should maybe split position / rotation into separate components.")
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
