#pragma once

#include "vob/aoe/rendering/LightType.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <entt/entt.hpp>


namespace vob::aoegl
{
	struct LightComponent
	{
		LightType type = LightType::Point;
		float radius = 10.0f;
		float intensity = 10.0f;
		glm::vec3 color = glm::vec3{ 1.0f };
		float innerAngle = 0.0f;
		float outerAngle = 0.0f;
		bool castsShadow = false;
		float size = 0.5f;
		float nearClip = 0.1f;
	};
}
