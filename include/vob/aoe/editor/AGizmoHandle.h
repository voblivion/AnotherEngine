#pragma once

#include "glm/glm.hpp"


namespace vob::aoedi
{
	struct AGizmoHandle
	{
		virtual ~AGizmoHandle() = default;

		virtual float getDistance(glm::vec3 const& a_position, glm::vec3 const& a_direction) const = 0;
		virtual void beginDrag(glm::vec3 const& a_position, glm::vec3 const& a_direction) = 0;
		virtual void drag(glm::vec3 const& a_position, glm::vec3 const& a_direction) = 0;
		virtual void endDrag() = 0;
	};
}
