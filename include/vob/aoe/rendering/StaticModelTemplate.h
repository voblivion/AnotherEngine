#pragma once

#include "vob/aoe/rendering/resources/GpuMaterial.h"
#include "vob/aoe/rendering/resources/GpuMesh.h"
#include "vob/aoe/rendering/resources/Handle.h"
#include "vob/aoe/rendering/ShadingPass.h"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"


namespace vob::aoegl
{
	struct StaticModelTemplate
	{
		struct Mesh
		{
			Handle<GpuMesh> mesh;
			Handle<GpuMaterial> material;
		};

		struct Light
		{
			glm::vec3 position = glm::vec3{ 0.0f };
			glm::quat rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };
			aoegl::LightType type = aoegl::LightType::Point;
			float radius = 50.0f;
			float intensity = 1.0f;
			glm::vec3 color;
			float innerAngle = 0.5f;
			float outerAngle = 0.75f;
			bool castsShadow = false;
			float size = 0.5f;
			float nearClip = 0.1f;
		};

		std::vector<Mesh> meshes;
		float boundingRadius = 0.0f;
		std::vector<Light> lights;
	};
}
