#pragma once

#include <vob/aoe/rendering/color.h>
#include <vob/aoe/rendering/primitives.h>

#include <vob/aoe/spacetime/measures.h>
#include <vob/aoe/spacetime/transform.h>


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

		void add_ellipsoid(
			glm::mat4 const& a_transform,
			glm::vec3 const& a_radiuses,
			aoegl::rgba const& a_color,
			int32_t a_horizontalSliceCount = 3,
			int32_t a_horizontalSliceSubdivisionCount = 4,
			int32_t a_verticalSliceCount = 4,
			int32_t a_verticalSliceSubidivisionCount = 4)
		{

			for (int h = 0; h < a_horizontalSliceCount; ++h)
			{
				auto const hSliceAngle0 = (static_cast<float>(h) / (a_horizontalSliceCount + 1) - 0.5f) * std::numbers::pi_v<float>;
				auto const hSliceAngle1 = (static_cast<float>(h + 1) / (a_horizontalSliceCount + 1) - 0.5f) * std::numbers::pi_v<float>;
				for (int hs = 0; hs < a_horizontalSliceSubdivisionCount; ++hs)
				{
					auto const hSubR0 = static_cast<float>(hs) / a_horizontalSliceSubdivisionCount;
					auto const hSubR1 = static_cast<float>(hs + 1) / a_horizontalSliceSubdivisionCount;
					auto const hSubAngle0 = hSliceAngle0 + hSubR0 * (hSliceAngle1 - hSliceAngle0);
					auto const hSubAngle1 = hSliceAngle0 + hSubR1 * (hSliceAngle1 - hSliceAngle0);

					auto const y0 = a_radiuses.y * std::sin(hSubAngle0);
					auto const r0 = std::cos(hSubAngle0);
					auto const y1 = a_radiuses.y * std::sin(hSubAngle1);
					auto const r1 = std::cos(hSubAngle1);

					for (int v = 0; v < 2 * a_verticalSliceCount; ++v)
					{
						auto const vSliceAngle = (static_cast<float>(v) / a_verticalSliceCount) * std::numbers::pi_v<float>;
						auto const vSliceCos = std::cos(vSliceAngle);
						auto const vSliceSin = std::sin(vSliceAngle);
						auto const localPos0 = glm::vec3{ r0 * a_radiuses.x * vSliceSin, y0, r0 * a_radiuses.z * vSliceCos };
						auto const localPos1 = glm::vec3{ r1 * a_radiuses.x * vSliceSin, y1, r1 * a_radiuses.z * vSliceCos };
						add_line(aoest::apply(a_transform, localPos0), aoest::apply(a_transform, localPos1), a_color);
					}
				}

				for (int v = 0; v < 2 * a_verticalSliceCount; ++v)
				{
					auto const vSliceAngle0 = (static_cast<float>(v) / a_verticalSliceCount) * std::numbers::pi_v<float>;
					auto const vSliceAngle1 = (static_cast<float>(v + 1) / a_verticalSliceCount) * std::numbers::pi_v<float>;

					for (int vs = 0; vs < a_verticalSliceSubidivisionCount; ++vs)
					{
						auto const r = std::cos(hSliceAngle1);
						auto const y = a_radiuses.y * std::sin(hSliceAngle1);

						auto const vSubR0 = static_cast<float>(vs) / a_verticalSliceSubidivisionCount;
						auto const vSubR1 = static_cast<float>(vs + 1) / a_verticalSliceSubidivisionCount;
						auto const vSubAngle0 = vSliceAngle0 + vSubR0 * (vSliceAngle1 - vSliceAngle0);
						auto const vSubAngle1 = vSliceAngle0 + vSubR1 * (vSliceAngle1 - vSliceAngle0);

						auto const vSubCos0 = std::cos(vSubAngle0);
						auto const vSubSin0 = std::sin(vSubAngle0);
						auto const vSubCos1 = std::cos(vSubAngle1);
						auto const vSubSin1 = std::sin(vSubAngle1);
						auto const localPos0 = glm::vec3{ r * a_radiuses.x * vSubSin0, y, r * a_radiuses.z * vSubCos0 };
						auto const localPos1 = glm::vec3{ r * a_radiuses.x * vSubSin1, y, r * a_radiuses.z * vSubCos1 };
						add_line(aoest::apply(a_transform, localPos0), aoest::apply(a_transform, localPos1), a_color);
					}
				}
			}

			auto const hSliceAngle0 = (static_cast<float>(a_horizontalSliceCount) / (a_horizontalSliceCount + 1) - 0.5f) * std::numbers::pi_v<float>;
			auto const hSliceAngle1 = (static_cast<float>(a_horizontalSliceCount + 1) / (a_horizontalSliceCount + 1) - 0.5f) * std::numbers::pi_v<float>;
			for (int hs = 0; hs < a_horizontalSliceSubdivisionCount; ++hs)
			{
				auto const hSubR0 = static_cast<float>(hs) / a_horizontalSliceSubdivisionCount;
				auto const hSubR1 = static_cast<float>(hs + 1) / a_horizontalSliceSubdivisionCount;
				auto const hSubAngle0 = hSliceAngle0 + hSubR0 * (hSliceAngle1 - hSliceAngle0);
				auto const hSubAngle1 = hSliceAngle0 + hSubR1 * (hSliceAngle1 - hSliceAngle0);

				auto const y0 = a_radiuses.y * std::sin(hSubAngle0);
				auto const r0 = std::cos(hSubAngle0);
				auto const y1 = a_radiuses.y * std::sin(hSubAngle1);
				auto const r1 = std::cos(hSubAngle1);

				for (int v = 0; v < 2 * a_verticalSliceCount; ++v)
				{
					auto const vSliceAngle = (static_cast<float>(v) / a_verticalSliceCount) * std::numbers::pi_v<float>;
					auto const vSliceCos = std::cos(vSliceAngle);
					auto const vSliceSin = std::sin(vSliceAngle);
					auto const localPos0 = glm::vec3{ r0 * a_radiuses.x * vSliceSin, y0, r0 * a_radiuses.z * vSliceCos };
					auto const localPos1 = glm::vec3{ r1 * a_radiuses.x * vSliceSin, y1, r1 * a_radiuses.z * vSliceCos };
					add_line(aoest::apply(a_transform, localPos0), aoest::apply(a_transform, localPos1), a_color);
				}
			}
		}

		void add_aabb(glm::vec3 const& a_min, glm::vec3 const& a_max, rgba a_color)
		{
			add_line(glm::vec3{ a_min.x, a_min.y, a_min.z }, glm::vec3{ a_min.x, a_min.y, a_max.z }, a_color);
			add_line(glm::vec3{ a_min.x, a_min.y, a_max.z }, glm::vec3{ a_max.x, a_min.y, a_max.z }, a_color);
			add_line(glm::vec3{ a_max.x, a_min.y, a_max.z }, glm::vec3{ a_max.x, a_min.y, a_min.z }, a_color);
			add_line(glm::vec3{ a_max.x, a_min.y, a_min.z }, glm::vec3{ a_min.x, a_min.y, a_min.z }, a_color);

			add_line(glm::vec3{ a_min.x, a_min.y, a_min.z }, glm::vec3{ a_min.x, a_max.y, a_min.z }, a_color);
			add_line(glm::vec3{ a_min.x, a_min.y, a_max.z }, glm::vec3{ a_min.x, a_max.y, a_max.z }, a_color);
			add_line(glm::vec3{ a_max.x, a_min.y, a_max.z }, glm::vec3{ a_max.x, a_max.y, a_max.z }, a_color);
			add_line(glm::vec3{ a_max.x, a_min.y, a_min.z }, glm::vec3{ a_max.x, a_max.y, a_min.z }, a_color);

			add_line(glm::vec3{ a_min.x, a_max.y, a_min.z }, glm::vec3{ a_min.x, a_max.y, a_max.z }, a_color);
			add_line(glm::vec3{ a_min.x, a_max.y, a_max.z }, glm::vec3{ a_max.x, a_max.y, a_max.z }, a_color);
			add_line(glm::vec3{ a_max.x, a_max.y, a_max.z }, glm::vec3{ a_max.x, a_max.y, a_min.z }, a_color);
			add_line(glm::vec3{ a_max.x, a_max.y, a_min.z }, glm::vec3{ a_min.x, a_max.y, a_min.z }, a_color);
		}

		void add_sphere(glm::vec3 const& a_position,
			float a_radius,
			aoegl::rgba const& a_color,
			int32_t a_horizontalSliceCount = 3,
			int32_t a_horizontalSliceSubdivisionCount = 4,
			int32_t a_verticalSliceCount = 4,
			int32_t a_verticalSliceSubidivisionCount = 4)
		{
			add_ellipsoid(
				aoest::combine(a_position, glm::quat{}),
				glm::vec3{ a_radius },
				a_color,
				a_horizontalSliceCount,
				a_horizontalSliceSubdivisionCount,
				a_verticalSliceCount,
				a_verticalSliceSubidivisionCount);
		}

		void add_triangle(
			glm::vec3 const& a_p0,
			glm::vec3 const& a_p1,
			glm::vec3 const& a_p2,
			aoegl::rgba const& a_color,
			int32_t a_subdivisionLength = 20.0f)
		{
			add_line(a_p0, a_p1, a_color);
			add_line(a_p1, a_p2, a_color);

			auto const s01Length = glm::length(a_p1 - a_p0);
			auto const s12Length = glm::length(a_p2 - a_p1);

			auto const subdivisionCount = static_cast<std::int32_t>(std::ceil(s01Length / a_subdivisionLength));
			for (auto subdivisionIndex = 0; subdivisionIndex < subdivisionCount; ++subdivisionIndex)
			{
				auto const subdivisionRatio = static_cast<float>(subdivisionIndex) / subdivisionCount;

				add_line(a_p2 + (a_p1 - a_p2) * subdivisionRatio, a_p0 + (a_p1 - a_p0) * subdivisionRatio, a_color);
			}
		}

		void clear_lines()
		{
			m_vertices.clear();
			m_lines.clear();
		}
	};
}
