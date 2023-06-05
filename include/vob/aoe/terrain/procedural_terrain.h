#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/data/mesh_data.h>

#include <vob/misc/std/vector2d.h>

#include <glm/glm.hpp>

#include <span>


namespace vob::aoetr
{
	struct layer
	{
		float m_height;
		float m_frequency;
		glm::vec2 m_offset = glm::vec2{ 0.0f };
	};

	mistd::vector2d<float> VOB_AOE_API generate_procedural_heights(
		glm::vec2 a_center, glm::vec2 a_size, glm::ivec2 a_subdivisions, std::span<layer> a_layers);

	mistd::vector2d<glm::vec3> VOB_AOE_API generated_procedural_positions(
		glm::vec2 a_center, glm::vec2 a_size, glm::ivec2 a_subdivisions, std::span<layer> a_layers);

	aoegl::mesh_data VOB_AOE_API generate_procedural_mesh(
		glm::vec2 a_center, glm::vec2 a_size, glm::ivec2 a_subdivisions, std::span<layer> a_layers, bool a_useSmoothShading);
}
