#pragma once

#include <assimp/vector3.h>
#include <assimp/vector2.h>
#include <glm/fwd.hpp>

namespace aoe
{
	namespace common
	{
		inline glm::vec3 toGlmVec3(aiVector3D const& a_vector)
		{
			return glm::vec3{ a_vector.x, a_vector.y, a_vector.z };
		}

		inline glm::vec2 toGlmVec2(aiVector2D const& a_vector)
		{
			return glm::vec2{ a_vector.x, a_vector.y };
		}

		inline glm::vec2 toGlmVec2(aiVector3D const& a_vector)
		{
			return glm::vec2{ a_vector.x, a_vector.y };
		}
	}
}