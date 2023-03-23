#pragma once

#include <vob/aoe/rendering/color.h>
#include <vob/aoe/rendering/primitives.h>

#include <vob/aoe/spacetime/measures.h>


namespace vob::aoegl
{
	struct debug_vertex
	{
		aoest::length_vector m_position;
		rgba m_color = rgba{ color_channel{ 1.0f } };
	};

	struct debug_mesh_world_component
	{
		std::pmr::vector<debug_vertex> m_vertices;
		std::pmr::vector<line> m_lines;

		void add_line(debug_vertex a_source, debug_vertex a_target)
		{
			auto const v0 = static_cast<std::uint32_t>(m_vertices.size());
			m_vertices.emplace_back(a_source);
			auto const v1 = static_cast<std::uint32_t>(m_vertices.size());
			m_vertices.emplace_back(a_target);
		}

		void clear_lines()
		{
			m_vertices.clear();
			m_lines.clear();
		}
	};
}
