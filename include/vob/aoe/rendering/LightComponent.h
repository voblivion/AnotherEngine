#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <entt/entt.hpp>


namespace vob::aoegl
{
	struct LightComponent
	{
		enum class Type
		{
			Point,
			Spot
		};

		Type type = Type::Point;
		float radius = 10.0f;
		float intensity = 10.0f;
		glm::vec3 color = glm::vec3{ 1.0f };
		float innerAngle = 0.0f;
		float outerAngle = 0.0f;
		float shadowLightSize = 0.5f;
	};
}
