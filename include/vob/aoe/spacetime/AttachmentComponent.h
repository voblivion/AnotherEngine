#pragma once

#include <entt/entt.hpp>
#include <glm/fwd.hpp>


namespace vob::aoest
{
	struct AttachmentComponent
	{
		entt::entity target = entt::null;
		glm::vec3 offsetPosition = glm::vec3{ 0.0f };
		glm::quat offsetRotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };
	};
}
