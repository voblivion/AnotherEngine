#pragma once

#include <vob/aoe/rendering/Color.h>
#include <vob/aoe/rendering/GraphicTypes.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <numbers>
#include <vector>


namespace vob::aoegl
{
	struct DebugVertex
	{
		glm::vec3 position;
		Rgba color = Rgba{ ColorChannel{ 1.0f } };
	};

	struct DebugMeshContext
	{
		struct DebugLine
		{
			GraphicIndex v0;
			GraphicIndex v1;
		};

		std::vector<DebugVertex> vertices;
		std::vector<DebugLine> lines;

		void clear()
		{
			vertices.clear();
			lines.clear();
		}

		void addLine(DebugVertex const& a_source, DebugVertex const& a_target)
		{
			auto const v0 = static_cast<GraphicIndex>(vertices.size());
			vertices.push_back(a_source);
			auto const v1 = static_cast<GraphicIndex>(vertices.size());
			vertices.push_back(a_target);
			lines.emplace_back(v0, v1);
		}

		void addLine(glm::vec3 const& a_source, glm::vec3 const& a_target, Rgba const& a_color)
		{
			addLine(DebugVertex{ a_source, a_color }, DebugVertex{ a_target, a_color });
		}

		void addTriangle(glm::vec3 const& a_p0, glm::vec3 const& a_p1, glm::vec3 const& a_p2, Rgba const& a_color)
		{
			// TODO: add subdivisions
			static constexpr float k_subdivisionLength = 1.0f;
			auto const d0 = glm::length(a_p1 - a_p0);
			auto const d1 = glm::length(a_p2 - a_p1);
			auto const d2 = glm::length(a_p0 - a_p2);

			auto const p0 = d0 < d1 ? (d0 < d2 ? a_p2 : a_p1) : (d1 < d2 ? a_p0 : a_p1);
			auto const p1 = d0 < d1 ? (d0 < d2 ? a_p0 : a_p2) : (d1 < d2 ? a_p1 : a_p2);
			auto const p2 = d0 < d1 ? (d0 < d2 ? a_p1 : a_p0) : (d1 < d2 ? a_p2 : a_p0);
			auto const d = d0 > d1 ? (d0 > d2 ? d0 : d2) : (d1 > d2 ? d1 : d2);

			auto const subdivisions = static_cast<int32_t>(d);

			addLine(p1, p2, a_color);

			for (int i = 0; i < subdivisions; ++i)
			{
				auto const q1 = p1 + static_cast<float>(i) / (subdivisions + 1) * (p0 - p1);
				auto const r1 = p1 + static_cast<float>(i + 1) / (subdivisions + 1) * (p0 - p1);
				auto const q2 = p2 + static_cast<float>(i) / (subdivisions + 1) * (p0 - p2);
				auto const r2 = p2 + static_cast<float>(i + 1) / (subdivisions + 1) * (p0 - p2);

				addLine(q1, r1, a_color);
				addLine(r1, r2, a_color);
				addLine(r2, q2, a_color);
				addLine(q2, q1, a_color);
			}

			auto const r1 = p1 + static_cast<float>(subdivisions) / (subdivisions + 1) * (p0 - p1);
			auto const r2 = p2 + static_cast<float>(subdivisions) / (subdivisions + 1) * (p0 - p2);

			addLine(r1, p0, a_color);
			addLine(r2, p0, a_color);
		}

		void addEllipsoid(glm::vec3 const& a_position, glm::quat const& a_rotation, glm::vec3 const& a_radiuses, aoegl::Rgba const& a_color)
		{
			static constexpr int k_horizontalSliceCount = 3;
			static constexpr int k_horizontalSliceSubdivisionCount = 4;
			static constexpr int k_verticalSliceCount = 4;
			static constexpr int k_verticalSliceSubdivisionCount = 4;

			for (int h = 0; h < k_horizontalSliceCount; ++h)
			{
				auto const hSliceAngle0 = (static_cast<float>(h) / (k_horizontalSliceCount + 1) - 0.5f) * std::numbers::pi_v<float>;
				auto const hSliceAngle1 = (static_cast<float>(h + 1) / (k_horizontalSliceCount + 1) - 0.5f) * std::numbers::pi_v<float>;
				for (int hs = 0; hs < k_horizontalSliceSubdivisionCount; ++hs)
				{
					auto const hSubR0 = static_cast<float>(hs) / k_horizontalSliceSubdivisionCount;
					auto const hSubR1 = static_cast<float>(hs + 1) / k_horizontalSliceSubdivisionCount;
					auto const hSubAngle0 = hSliceAngle0 + hSubR0 * (hSliceAngle1 - hSliceAngle0);
					auto const hSubAngle1 = hSliceAngle0 + hSubR1 * (hSliceAngle1 - hSliceAngle0);

					auto const y0 = a_radiuses.y * std::sin(hSubAngle0);
					auto const r0 = std::cos(hSubAngle0);
					auto const y1 = a_radiuses.y * std::sin(hSubAngle1);
					auto const r1 = std::cos(hSubAngle1);

					for (int v = 0; v < 2 * k_verticalSliceCount; ++v)
					{
						auto const vSliceAngle = (static_cast<float>(v) / k_verticalSliceCount) * std::numbers::pi_v<float>;
						auto const vSliceCos = std::cos(vSliceAngle);
						auto const vSliceSin = std::sin(vSliceAngle);
						auto const localPos0 = glm::vec3{ r0 * a_radiuses.x * vSliceSin, y0, r0 * a_radiuses.z * vSliceCos };
						auto const localPos1 = glm::vec3{ r1 * a_radiuses.x * vSliceSin, y1, r1 * a_radiuses.z * vSliceCos };
						addLine(a_position + a_rotation * localPos0, a_position + a_rotation * localPos1, a_color);
					}
				}

				for (int v = 0; v < 2 * k_verticalSliceCount; ++v)
				{
					auto const vSliceAngle0 = (static_cast<float>(v) / k_verticalSliceCount) * std::numbers::pi_v<float>;
					auto const vSliceAngle1 = (static_cast<float>(v + 1) / k_verticalSliceCount) * std::numbers::pi_v<float>;

					for (int vs = 0; vs < k_verticalSliceSubdivisionCount; ++vs)
					{
						auto const r = std::cos(hSliceAngle1);
						auto const y = a_radiuses.y * std::sin(hSliceAngle1);

						auto const vSubR0 = static_cast<float>(vs) / k_verticalSliceSubdivisionCount;
						auto const vSubR1 = static_cast<float>(vs + 1) / k_verticalSliceSubdivisionCount;
						auto const vSubAngle0 = vSliceAngle0 + vSubR0 * (vSliceAngle1 - vSliceAngle0);
						auto const vSubAngle1 = vSliceAngle0 + vSubR1 * (vSliceAngle1 - vSliceAngle0);

						auto const vSubCos0 = std::cos(vSubAngle0);
						auto const vSubSin0 = std::sin(vSubAngle0);
						auto const vSubCos1 = std::cos(vSubAngle1);
						auto const vSubSin1 = std::sin(vSubAngle1);
						auto const localPos0 = glm::vec3{ r * a_radiuses.x * vSubSin0, y, r * a_radiuses.z * vSubCos0 };
						auto const localPos1 = glm::vec3{ r * a_radiuses.x * vSubSin1, y, r * a_radiuses.z * vSubCos1 };
						addLine(a_position + a_rotation * localPos0, a_position + a_rotation * localPos1, a_color);
					}
				}
			}

			auto const hSliceAngle0 = (static_cast<float>(k_horizontalSliceCount) / (k_horizontalSliceCount + 1) - 0.5f) * std::numbers::pi_v<float>;
			auto const hSliceAngle1 = (static_cast<float>(k_horizontalSliceCount + 1) / (k_horizontalSliceCount + 1) - 0.5f) * std::numbers::pi_v<float>;
			for (int hs = 0; hs < k_horizontalSliceSubdivisionCount; ++hs)
			{
				auto const hSubR0 = static_cast<float>(hs) / k_horizontalSliceSubdivisionCount;
				auto const hSubR1 = static_cast<float>(hs + 1) / k_horizontalSliceSubdivisionCount;
				auto const hSubAngle0 = hSliceAngle0 + hSubR0 * (hSliceAngle1 - hSliceAngle0);
				auto const hSubAngle1 = hSliceAngle0 + hSubR1 * (hSliceAngle1 - hSliceAngle0);

				auto const y0 = a_radiuses.y * std::sin(hSubAngle0);
				auto const r0 = std::cos(hSubAngle0);
				auto const y1 = a_radiuses.y * std::sin(hSubAngle1);
				auto const r1 = std::cos(hSubAngle1);

				for (int v = 0; v < 2 * k_verticalSliceCount; ++v)
				{
					auto const vSliceAngle = (static_cast<float>(v) / k_verticalSliceCount) * std::numbers::pi_v<float>;
					auto const vSliceCos = std::cos(vSliceAngle);
					auto const vSliceSin = std::sin(vSliceAngle);
					auto const localPos0 = glm::vec3{ r0 * a_radiuses.x * vSliceSin, y0, r0 * a_radiuses.z * vSliceCos };
					auto const localPos1 = glm::vec3{ r1 * a_radiuses.x * vSliceSin, y1, r1 * a_radiuses.z * vSliceCos };
					addLine(a_position + a_rotation * localPos0, a_position + a_rotation * localPos1, a_color);
				}
			}
		}

		void addSphere(glm::vec3 const& a_position, float a_radius, aoegl::Rgba const& a_color)
		{
			addEllipsoid(a_position, glm::quat{}, glm::vec3{ a_radius }, a_color);
		}
	};
}
