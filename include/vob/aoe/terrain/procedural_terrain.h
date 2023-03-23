#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/data/mesh_data.h>

#include <glm/glm.hpp>

#include <span>


namespace vob::aoetr
{
	aoegl::mesh_data VOB_AOE_API generate_procedural_terrain(
		std::int32_t a_size, float a_cellSize, float a_height, float a_f);

	struct layer
	{
		float m_height;
		float m_frequency;
		glm::vec2 m_offset = glm::vec2{ 0.0f };
	};

	aoegl::mesh_data VOB_AOE_API generate_procedural_terrain(
		float a_size, float a_cellSize, std::span<layer> a_layers, bool a_useSmoothShading);
}
