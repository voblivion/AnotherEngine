#pragma once

#include <vob/aoe/rendering/GraphicTypes.h>

#include <glm/glm.hpp>

#include <vector>


namespace vob::aoeph
{
	struct alignas(16) WheelRenderSceneConfig
	{
		glm::mat4 wheel;
		glm::vec3 albedo;
		float metallic;
		float roughness;
		float steering;
		float driving;
	};

	struct CarMaterialsComponent
	{
		struct Part
		{
			int32_t wheelIndex = 0;
			glm::vec3 albedo = glm::vec3{ 1.0f };
			float metallic;
			float roughness;
			aoegl::GraphicId materialUbo = aoegl::k_invalidId;
			float distance = 0.0f;
		};

		std::vector<Part> parts;
	};
}
