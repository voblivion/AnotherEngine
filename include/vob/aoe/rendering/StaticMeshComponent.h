#pragma once

#include <vob/aoe/rendering/GraphicTypes.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <entt/entt.hpp>

#include <algorithm>
#include <memory>
#include <span>
#include <unordered_map>
#include <vector>


namespace vob::aoegl
{
	struct SceneMeshRenderingData
	{
		alignas(16) glm::mat4 viewProjectionMatrix;
		alignas(16) glm::vec3 cameraPosition;
		// other data that will always come with rendering in a scene
	};

	struct StaticMeshComponent
	{
		struct Part
		{
			GraphicId materialProgram;
			GraphicId instanceUbo;
			GraphicId meshVao;
			int32_t indexCount;
		};

		std::vector<Part> parts;

		// TODO: could improve by replacing with obb (or even aabb for static elements?)
		float boundingRadius = 0.0f;
	};
}
