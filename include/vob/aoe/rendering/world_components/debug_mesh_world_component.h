#pragma once

#include <vob/aoe/rendering/color.h>
#include <vob/aoe/rendering/primitives.h>

#include <vob/aoe/spacetime/measures.h>


namespace vob::aoegl
{
	struct debug_vertex
	{
		glm::vec3 m_position;
		rgba m_color = rgba{ color_channel{ 1.0f } };
	};

	struct debug_mesh_world_component
	{
		std::pmr::vector<debug_vertex> m_vertices;
		std::pmr::vector<line> m_lines;

		void add_line(glm::vec3 a_source, glm::vec3 a_target, rgba a_color)
		{
			add_line(debug_vertex{ a_source, a_color }, debug_vertex{ a_target, a_color });
		}

		void add_line(debug_vertex a_source, debug_vertex a_target)
		{
			auto const v0 = static_cast<std::uint32_t>(m_vertices.size());
			m_vertices.emplace_back(a_source);
			auto const v1 = static_cast<std::uint32_t>(m_vertices.size());
			m_vertices.emplace_back(a_target);
			m_lines.emplace_back(v0, v1);
		}

		void clear_lines()
		{
			m_vertices.clear();
			m_lines.clear();
		}
	};
}
