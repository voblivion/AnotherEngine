#include "vob/aoe/debug/DebugDevblogSystem.h"

#include "vob/aoe/physics/Shapes.h"

#include "imgui.h"

#include <optional>


#pragma optimize("", off)
namespace vob::aoedb
{
	void DebugDevblogSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{

	}

	void DebugDevblogSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		static int32_t k_step = 0;

		if (ImGui::Begin("Devblog"))
		{
			if (ImGui::Button("Reset"))
			{
				k_step = 0;
			}

			if (ImGui::Button("Back"))
			{
				k_step -= 1;
			}

			if (ImGui::Button("Step"))
			{
				k_step += 1;
			}
		}
		ImGui::End();

		auto& debugMeshCtx = m_debugMeshContext.get(a_wdap);
		auto const dx = glm::normalize(glm::vec3{ 5.0f, 0.0f, 1.0f });
		auto const dy = glm::cross(glm::vec3{ 0.0f, 1.0f, 0.0f }, dx);
		auto const p = glm::vec3{ 300.0f, 0.0f, 0.0f };

		std::vector<aoeph::Triangle> triangles;
		triangles.reserve(8 * 2 * 2);
		for (int32_t i = 0; i < 8; ++i)
		{
			auto const p0 = p + (2.0f * i) * dx;
			auto const p1 = p + (2.0f * (i + 1)) * dx;
			auto const p2 = p0 + 10.0f * dy;
			auto const p3 = p1 + 10.0f * dy;
			auto const p4 = p2 + 2.0f * dy;
			auto const p5 = p3 + 2.0f * dy;

			triangles.emplace_back(p0, p1, p2);
			triangles.emplace_back(p3, p2, p1);
			triangles.emplace_back(p2, p3, p4);
			triangles.emplace_back(p5, p4, p3);
		}
		auto const triangleCount = 8 * 2 * 2;

		auto const triangleSpan = [&](int32_t beginIndex, int32_t count)
			{
				return std::span{ &triangles[beginIndex], static_cast<size_t>(count) };
			};

		auto const drawT = [&](std::span<aoeph::Triangle const> triangles, glm::vec3 const& color, bool drawCenter = false, float scale = 1.0f)
			{
				for (auto const& triangle : triangles)
				{
					auto const c = (triangle.p0 + triangle.p1 + triangle.p2) / 3.0f;
					debugMeshCtx.addTriangle(c + scale * (triangle.p0 - c), c + scale * (triangle.p1 - c), c + scale * (triangle.p2 - c), aoegl::toRgba(color), 0);
					if (drawCenter)
					{
						debugMeshCtx.addSphere(c, 0.1f, aoegl::toRgba(color), 1, 4, 2, 4);
					}
				}
			};

		auto const drawB = [&](aoeph::Aabb const& bounds, glm::vec3 const& color, float const scale = 1.0f)
			{
				auto const p0 = bounds.min;
				auto const p1 = glm::vec3{ bounds.min.x, 0.0f, bounds.max.z };
				auto const p2 = bounds.max;
				auto const p3 = glm::vec3{ bounds.max.x, 0.0f, bounds.min.z };

				debugMeshCtx.addLine(p0, p1, aoegl::toRgba(color));
				debugMeshCtx.addLine(p1, p2, aoegl::toRgba(color));
				debugMeshCtx.addLine(p2, p3, aoegl::toRgba(color));
				debugMeshCtx.addLine(p3, p0, aoegl::toRgba(color));
			};

		auto const drawN = [&](std::optional<glm::vec3> const& parent, glm::vec3 point, glm::vec3 const& color)
			{
				debugMeshCtx.addSphere(point, 0.5f, aoegl::toRgba(color), 1, 4, 2, 4);
				if (parent != std::nullopt)
				{
					debugMeshCtx.addLine(*parent, point, aoegl::toRgba(color));
				}
			};

		auto const drawL = [&](glm::vec3 const& point, std::span<aoeph::Triangle const> triangles, glm::vec3 const& color)
			{
				for (auto const& triangle : triangles)
				{
					auto const c = (triangle.p0 + triangle.p1 + triangle.p2) / 3.0f;
					debugMeshCtx.addLine(c, point, aoegl::toRgba(color));
				}
			};


		auto const computeBounds = [&](std::span<aoeph::Triangle> triangles)
			{
				auto bounds = aoeph::Aabb{ glm::vec3{std::numeric_limits<float>::infinity()}, glm::vec3{-std::numeric_limits<float>::infinity()} };
				for (auto const& triangle : triangles)
				{
					bounds.min = glm::min(triangle.p0, bounds.min);
					bounds.min = glm::min(triangle.p1, bounds.min);
					bounds.min = glm::min(triangle.p2, bounds.min);
					bounds.max = glm::max(triangle.p0, bounds.max);
					bounds.max = glm::max(triangle.p1, bounds.max);
					bounds.max = glm::max(triangle.p2, bounds.max);
				}
				return bounds;
			};

		auto const bvhSplit = [&](std::span<aoeph::Triangle> triangles)
			{
				auto const bounds = computeBounds(triangles);
				auto const extents = bounds.max - bounds.min;
				auto const mainAxis = extents.x > extents.z ? 0 : 2;
				std::sort(triangles.begin(), triangles.end(), [mainAxis](aoeph::Triangle const& lhs, aoeph::Triangle const& rhs) {
					return lhs.p0[mainAxis] + lhs.p1[mainAxis] + lhs.p2[mainAxis] < rhs.p0[mainAxis] + rhs.p1[mainAxis] + rhs.p2[mainAxis]; });

				return std::pair{
					std::span{&triangles[0], triangles.size() / 2},
					std::span{&triangles[0] + triangles.size() / 2, triangles.size() - triangles.size() / 2}
				};
			};

		auto step = k_step;

		if (step-- == 0)
		{
			drawT(triangles, aoegl::k_gray);
		}

		if (step-- == 0)
		{
			drawT(triangles, aoegl::k_white);
			auto const bounds = computeBounds(triangles);
			drawB(bounds, aoegl::k_magenta);
		}

		if (step-- == 0)
		{
			auto const bounds = computeBounds(triangles);
			drawB(bounds, aoegl::k_magenta);
			auto const [triangles0, triangles1] = bvhSplit(triangles);
			drawT(triangles0, aoegl::k_red, true);
			drawT(triangles1, aoegl::k_blue, true);
		}

		if (step-- == 0)
		{
			auto const [triangles0, triangles1] = bvhSplit(triangles);

			drawT(triangles0, 0.5f * glm::vec3{ aoegl::k_red }, true);
			drawT(triangles1, 0.5f * glm::vec3{ aoegl::k_blue }, true);

			auto const bounds0 = computeBounds(triangles0);
			drawB(bounds0, aoegl::k_red, true);
			auto const bounds1 = computeBounds(triangles1);
			drawB(bounds1, aoegl::k_blue, true);
		}

		if (step-- == 0)
		{
			auto const [triangles0, triangles1] = bvhSplit(triangles);
			drawT(triangles1, aoegl::k_gray);

			drawT(triangles0, aoegl::k_white);
			auto const bounds = computeBounds(triangles0);
			drawB(bounds, aoegl::k_magenta);
		}

		if (step-- == 0)
		{
			auto const [triangles0, triangles1] = bvhSplit(triangles);
			drawT(triangles1, aoegl::k_gray);

			auto const bounds = computeBounds(triangles0);
			drawB(bounds, aoegl::k_magenta);
			auto const [triangles00, triangles01] = bvhSplit(triangles0);
			drawT(triangles00, aoegl::k_red, true);
			drawT(triangles01, aoegl::k_blue, true);
		}

		if (step-- == 0)
		{
			auto const [triangles0, triangles1] = bvhSplit(triangles);
			drawT(triangles1, aoegl::k_gray);

			auto const [triangles00, triangles01] = bvhSplit(triangles0);
			drawT(triangles00, 0.5f * glm::vec3{ aoegl::k_red }, true);
			drawT(triangles01, 0.5f * glm::vec3{ aoegl::k_blue }, true);

			auto const bounds0 = computeBounds(triangles00);
			drawB(bounds0, aoegl::k_red, true);
			auto const bounds1 = computeBounds(triangles01);
			drawB(bounds1, aoegl::k_blue, true);
		}

		if (step-- == 0)
		{
			auto const [triangles0, triangles1] = bvhSplit(triangles);
			drawT(triangles1, aoegl::k_gray);

			auto const [triangles00, triangles01] = bvhSplit(triangles0);
			drawT(triangles01, aoegl::k_gray);

			drawT(triangles00, aoegl::k_white);
			auto const bounds = computeBounds(triangles00);
			drawB(bounds, aoegl::k_magenta);
		}

		if (step-- == 0)
		{
			auto const [triangles0, triangles1] = bvhSplit(triangles);
			drawT(triangles1, aoegl::k_gray);

			auto const [triangles00, triangles01] = bvhSplit(triangles0);
			drawT(triangles01, aoegl::k_gray);

			auto const bounds = computeBounds(triangles00);
			drawB(bounds, aoegl::k_magenta);
			auto const [triangles000, triangles001] = bvhSplit(triangles00);
			drawT(triangles000, aoegl::k_red, true);
			drawT(triangles001, aoegl::k_blue, true);
		}

		if (step-- == 0)
		{
			auto const [triangles0, triangles1] = bvhSplit(triangles);
			drawT(triangles1, aoegl::k_gray);

			auto const [triangles00, triangles01] = bvhSplit(triangles0);
			drawT(triangles01, aoegl::k_gray);

			auto const [triangles000, triangles001] = bvhSplit(triangles00);
			drawT(triangles000, 0.5f * glm::vec3{ aoegl::k_red }, true);
			drawT(triangles001, 0.5f * glm::vec3{ aoegl::k_blue }, true);

			auto const bounds0 = computeBounds(triangles000);
			drawB(bounds0, aoegl::k_red, true);
			auto const bounds1 = computeBounds(triangles001);
			drawB(bounds1, aoegl::k_blue, true);
		}

		auto const hsv = [](float h, float s, float v)
			{
				float r, g, b;
				int i = static_cast<int>(h * 6);
				float f = h * 6 - i;
				float p = v * (1 - s);
				float q = v * (1 - f * s);
				float t = v * (1 - (1 - f) * s);
				switch (i % 6)
				{
				case 0: r = v; g = t; b = p; break;
				case 1: r = q; g = v; b = p; break;
				case 2: r = p; g = v; b = t; break;
				case 3: r = p; g = q; b = v; break;
				case 4: r = t; g = p; b = v; break;
				case 5: r = v; g = p; b = q; break;
				}
				return glm::vec3{ r, g, b };
			};

		auto const rngColor = [&](int32_t i, int32_t n)
			{
				auto const goldenRatio = 0.618033988f;
				return hsv(std::fmod(i * goldenRatio, 1.0f), 1.0f, 1.0f);
			};

		auto const bounds = computeBounds(triangles);
		auto const top = glm::vec3{ (bounds.min.x + bounds.max.x) / 2.0f, 0.0f, bounds.min.z };
		auto const nodesZOffset = 2.0f;
		auto const nodesWidth = (bounds.max - bounds.min) / 2.0f;

		auto const computeNodePos = [&](int32_t depth, int32_t maxDepth, int32_t index)
			{
				auto const indicesAtDepth = 1 << depth;

				return top
					+ static_cast<float>(1.0f + maxDepth - depth) * nodesZOffset * glm::vec3{ 0.0f, 0.0f, -1.0f }
				+ ((0.5f + index) / indicesAtDepth - 0.5f) * nodesWidth * glm::vec3{ 1.0f, 0.0f, 0.0f };
			};

		if (step-- == 0)
		{
			drawT(triangles, aoegl::k_gray, true, 0.975f);

			drawN(std::nullopt, computeNodePos(0, 0, 0), aoegl::k_gray);
		}

		if (step-- == 0)
		{
			auto const [triangles0, triangles1] = bvhSplit(triangles);
			drawT(triangles0, rngColor(0, 2), true, 0.975f);
			drawT(triangles1, rngColor(1, 2), true, 0.975f);

			drawN(std::nullopt, computeNodePos(0, 1, 0), aoegl::k_gray);
			drawN(computeNodePos(0, 1, 0), computeNodePos(1, 1, 0), rngColor(0, 2));
			drawN(computeNodePos(0, 1, 0), computeNodePos(1, 1, 1), rngColor(1, 2));
		}

		if (step-- == 0)
		{
			auto const [triangles0, triangles1] = bvhSplit(triangles);
			auto const [triangles00, triangles01] = bvhSplit(triangles0);
			drawT(triangles00, rngColor(0, 4), true, 0.975f);
			drawT(triangles01, rngColor(1, 4), true, 0.975f);
			auto const [triangles10, triangles11] = bvhSplit(triangles1);
			drawT(triangles10, rngColor(2, 4), true, 0.975f);
			drawT(triangles11, rngColor(3, 4), true, 0.975f);

			drawN(std::nullopt, computeNodePos(0, 2, 0), aoegl::k_gray);
			drawN(computeNodePos(0, 2, 0), computeNodePos(1, 2, 0), aoegl::k_gray);
			drawN(computeNodePos(0, 2, 0), computeNodePos(1, 2, 1), aoegl::k_gray);
			drawN(computeNodePos(1, 2, 0), computeNodePos(2, 2, 0), rngColor(0, 4));
			drawN(computeNodePos(1, 2, 0), computeNodePos(2, 2, 1), rngColor(1, 4));
			drawN(computeNodePos(1, 2, 1), computeNodePos(2, 2, 2), rngColor(2, 4));
			drawN(computeNodePos(1, 2, 1), computeNodePos(2, 2, 3), rngColor(3, 4));
		}

		if (step-- == 0)
		{
			auto const [triangles0, triangles1] = bvhSplit(triangles);
			auto const [triangles00, triangles01] = bvhSplit(triangles0);
			auto const [triangles10, triangles11] = bvhSplit(triangles1);

			auto const [triangles000, triangles001] = bvhSplit(triangles00);
			drawT(triangles000, rngColor(0, 8), true, 0.975f);
			drawT(triangles001, rngColor(1, 8), true, 0.975f);
			auto const [triangles010, triangles011] = bvhSplit(triangles01);
			drawT(triangles010, rngColor(2, 8), true, 0.975f);
			drawT(triangles011, rngColor(3, 8), true, 0.975f);
			auto const [triangles100, triangles101] = bvhSplit(triangles10);
			drawT(triangles100, rngColor(4, 8), true, 0.975f);
			drawT(triangles101, rngColor(5, 8), true, 0.975f);
			auto const [triangles110, triangles111] = bvhSplit(triangles11);
			drawT(triangles110, rngColor(6, 8), true, 0.975f);
			drawT(triangles111, rngColor(7, 8), true, 0.975f);

			auto const pr = computeNodePos(0, 3, 0);
			auto const p0 = computeNodePos(1, 3, 0);
			auto const p1 = computeNodePos(1, 3, 1);
			auto const p00 = computeNodePos(2, 3, 0);
			auto const p01 = computeNodePos(2, 3, 1);
			auto const p10 = computeNodePos(2, 3, 2);
			auto const p11 = computeNodePos(2, 3, 3);
			auto const p000 = computeNodePos(3, 3, 0);
			auto const p001 = computeNodePos(3, 3, 1);
			auto const p010 = computeNodePos(3, 3, 2);
			auto const p011 = computeNodePos(3, 3, 3);
			auto const p100 = computeNodePos(3, 3, 4);
			auto const p101 = computeNodePos(3, 3, 5);
			auto const p110 = computeNodePos(3, 3, 6);
			auto const p111 = computeNodePos(3, 3, 7);

			drawN(std::nullopt, pr, aoegl::k_gray);
			drawN(pr, p0, aoegl::k_gray);
			drawN(pr, p1, aoegl::k_gray);
			drawN(p0, p00, aoegl::k_gray);
			drawN(p0, p01, aoegl::k_gray);
			drawN(p1, p10, aoegl::k_gray);
			drawN(p1, p11, aoegl::k_gray);
			drawN(p00, p000, rngColor(0, 8));
			drawN(p00, p001, rngColor(1, 8));
			drawN(p01, p010, rngColor(2, 8));
			drawN(p01, p011, rngColor(3, 8));
			drawN(p10, p100, rngColor(4, 8));
			drawN(p10, p101, rngColor(5, 8));
			drawN(p11, p110, rngColor(6, 8));
			drawN(p11, p111, rngColor(7, 8));
		}

		if (step-- == 0)
		{
			drawT(triangles, aoegl::k_gray, true, 0.975f);
		}

		if (step-- == 0)
		{
			auto const carPos = (bounds.max + bounds.min) / 2.0f;
			auto const carBounds = aoeph::Aabb{ carPos - glm::vec3{2.0f, 0.0f, 1.0f}, carPos + glm::vec3{2.0f, 0.0f, 1.0f} };

			drawT(triangles, aoegl::k_gray, true, 0.975f);
			drawB(carBounds, glm::vec3{ 0.0f, 0.5f, 0.0f });
		}

		if (step-- == 0)
		{

			auto const carPos = (bounds.max + bounds.min) / 2.0f;
			auto const carBounds = aoeph::Aabb{ carPos - glm::vec3{2.0f, 0.0f, 1.0f}, carPos + glm::vec3{2.0f, 0.0f, 1.0f} };
			auto const carExtendedBounds = aoeph::Aabb{ carPos - glm::vec3{5.0f, 0.0f, 4.0f}, carPos + glm::vec3{5.0f, 0.0f, 4.0f} };

			drawT(triangles, aoegl::k_gray, true, 0.975f);
			drawB(carBounds, glm::vec3{ 0.0f, 0.5f, 0.0f });
			drawB(carExtendedBounds, glm::vec3{ 0.0f, 1.0f, 0.0f });
		}

		auto const intersectT = [&](std::span<aoeph::Triangle> triangles, aoeph::Aabb const& bounds)
			{
				auto nonIntersectingIt = std::partition(triangles.begin(), triangles.end(), [&](aoeph::Triangle const& triangle)
					{
						auto const c = (bounds.max + bounds.min) / 2.0f;
						auto const h = (bounds.max - bounds.min) / 2.0f;
						auto const testAxis = [&](glm::vec2 const& n)
							{
								auto const p0 = glm::dot(n, glm::vec2{ triangle.p0.x, triangle.p0.z });
								auto const p1 = glm::dot(n, glm::vec2{ triangle.p1.x, triangle.p1.z });
								auto const p2 = glm::dot(n, glm::vec2{ triangle.p2.x, triangle.p2.z });
								auto const r = h.x * std::abs(n.x) + h.z * std::abs(n.y);
								auto const c2 = glm::dot(n, glm::vec2{ c.x, c.z });
								return !(c2 + r < std::min({ p0, p1, p2 }) || c2 - r > std::max({ p0, p1, p2 }));
							};
						auto const e0 = glm::vec2{ triangle.p1.x - triangle.p0.x, triangle.p1.z - triangle.p0.z };
						auto const e1 = glm::vec2{ triangle.p2.x - triangle.p1.x, triangle.p2.z - triangle.p1.z };
						auto const e2 = glm::vec2{ triangle.p0.x - triangle.p2.x, triangle.p0.z - triangle.p2.z };

						return testAxis({ 1.0f, 0.0f }) && testAxis({ 0.0f, 1.0f })
							&& testAxis({ -e0.y, e0.x }) && testAxis({ -e1.y, e1.x }) && testAxis({ -e2.y, e2.x });
					});

				auto const intersectingCount = std::distance(triangles.begin(), nonIntersectingIt);
				auto const nonIntersectingCount = std::distance(nonIntersectingIt, triangles.end());
				return std::pair{
					std::span{ &triangles[0], static_cast<size_t>(intersectingCount) },
					std::span{ &triangles[intersectingCount], static_cast<size_t>(nonIntersectingCount) }
				};
			};

		if (step-- == 0)
		{
			auto const carPos = (bounds.max + bounds.min) / 2.0f;
			auto const carBounds = aoeph::Aabb{ carPos - glm::vec3{2.0f, 0.0f, 1.0f}, carPos + glm::vec3{2.0f, 0.0f, 1.0f} };
			auto const carExtendedBounds = aoeph::Aabb{ carPos - glm::vec3{5.0f, 0.0f, 4.0f}, carPos + glm::vec3{5.0f, 0.0f, 4.0f} };

			auto const [intersectingTriangles, nonIntersectingTriangles] = intersectT(triangles, carExtendedBounds);
			drawT(intersectingTriangles, aoegl::k_yellow, true, 0.975f);
			drawT(nonIntersectingTriangles, aoegl::k_gray, true, 0.975f);
			drawB(carBounds, glm::vec3{ 0.0f, 0.5f, 0.0f });
			drawB(carExtendedBounds, glm::vec3{ 0.0f, 1.0f, 0.0f });
		}

		if (step-- == 0)
		{
			auto const carPos = (bounds.max + bounds.min) / 2.0f;
			auto const carBounds = aoeph::Aabb{ carPos - glm::vec3{2.0f, 0.0f, 1.0f}, carPos + glm::vec3{2.0f, 0.0f, 1.0f} };
			auto const carExtendedBounds = aoeph::Aabb{ carPos - glm::vec3{5.0f, 0.0f, 4.0f}, carPos + glm::vec3{5.0f, 0.0f, 4.0f} };

			auto const [intersectingTriangles, nonIntersectingTriangles] = intersectT(triangles, carExtendedBounds);
			drawT(intersectingTriangles, aoegl::k_white, true);
		}

		if (step-- == 0)
		{
			auto const [triangles0, triangles1] = bvhSplit(triangles);
			drawT(triangles1, aoegl::k_gray);

			auto const [triangles00, triangles01] = bvhSplit(triangles0);
			drawT(triangles00, 0.5f * glm::vec3{ aoegl::k_red }, true);
			drawT(triangles01, 0.5f * glm::vec3{ aoegl::k_blue }, true);

			auto const bounds0 = computeBounds(triangles00);
			drawB(bounds0, aoegl::k_red, true);
			auto const bounds1 = computeBounds(triangles01);
			drawB(bounds1, aoegl::k_blue, true);
		}

		if (step-- == 0)
		{
			auto const carPos = (bounds.max + bounds.min) / 2.0f;
			auto const carBounds = aoeph::Aabb{ carPos - glm::vec3{2.0f, 0.0f, 1.0f}, carPos + glm::vec3{2.0f, 0.0f, 1.0f} };
			auto const carExtendedBounds = aoeph::Aabb{ carPos - glm::vec3{5.0f, 0.0f, 4.0f}, carPos + glm::vec3{5.0f, 0.0f, 4.0f} };

			auto const [intersectingTriangles, nonIntersectingTriangles] = intersectT(triangles, carExtendedBounds);
			drawT(intersectingTriangles, aoegl::k_white, true);
		}

		auto const computeCenter = [&](std::span<aoeph::Triangle> triangles)
			{
				auto c = glm::vec3{ 0.0f };
				for (auto const& triangle : triangles)
				{
					c += triangle.p0 + triangle.p1 + triangle.p2;
				}
				return c / (3.0f * triangles.size());
			};

		if (step-- == 0)
		{
			auto const carPos = (bounds.max + bounds.min) / 2.0f;
			auto const carBounds = aoeph::Aabb{ carPos - glm::vec3{2.0f, 0.0f, 1.0f}, carPos + glm::vec3{2.0f, 0.0f, 1.0f} };
			auto const carExtendedBounds = aoeph::Aabb{ carPos - glm::vec3{5.0f, 0.0f, 4.0f}, carPos + glm::vec3{5.0f, 0.0f, 4.0f} };

			auto const [intersectingTriangles, nonIntersectingTriangles] = intersectT(triangles, carExtendedBounds);
			drawT(intersectingTriangles, aoegl::k_white, true);
			auto const center = computeCenter(intersectingTriangles);
			debugMeshCtx.addSphere(center, 0.25f, aoegl::toRgba(glm::vec3{0.0f, 1.0f, 0.0f}));
		}

		auto const computeCovariance = [&](std::span<aoeph::Triangle> triangles, glm::vec3 const& point)
			{
				auto cov = glm::mat3{ 0.0f };
				for (auto const& triangle : triangles)
				{
					auto const c = (triangle.p0 + triangle.p1 + triangle.p2) / 3.0f;
					cov += glm::outerProduct(point - c, point - c);
				}
				return cov;
			};

		auto const computeDominantEigenVector = [&](std::span<aoeph::Triangle> triangles, glm::vec3 const& point)
			{
				auto const cov = computeCovariance(triangles, point);

				auto dominantEigenVector = glm::length(cov * glm::vec3{ 1.0f, 0.0f, 0.0f }) > std::numeric_limits<float>::epsilon()
					? glm::vec3{ 1.0f, 0.0f, 0.0f } : glm::vec3{ 0.0f, 0.0f, 1.0f };
				for (int32_t k = 0; k < 20; ++k)
				{
					dominantEigenVector = glm::normalize(cov * dominantEigenVector);
				}
				return dominantEigenVector;
			};

		if (step-- == 0)
		{
			auto const carPos = (bounds.max + bounds.min) / 2.0f;
			auto const carBounds = aoeph::Aabb{ carPos - glm::vec3{2.0f, 0.0f, 1.0f}, carPos + glm::vec3{2.0f, 0.0f, 1.0f} };
			auto const carExtendedBounds = aoeph::Aabb{ carPos - glm::vec3{5.0f, 0.0f, 4.0f}, carPos + glm::vec3{5.0f, 0.0f, 4.0f} };

			auto const [intersectingTriangles, nonIntersectingTriangles] = intersectT(triangles, carExtendedBounds);
			drawT(intersectingTriangles, aoegl::k_white, true);
			auto const center = computeCenter(intersectingTriangles);
			debugMeshCtx.addSphere(center, 0.25f, aoegl::toRgba(glm::vec3{0.0f, 0.5f, 0.0f}));

			auto const dominantEigenVector = computeDominantEigenVector(intersectingTriangles, center);

			debugMeshCtx.addLine(center - 10.0f * dominantEigenVector, center + 10.0f * dominantEigenVector, aoegl::toRgba(glm::vec3{ 0.0f, 1.0f, 0.0f }));
		}

		auto const bspSplit = [&](std::span<aoeph::Triangle> triangles, glm::vec3 const& axis)
			{
				std::sort(triangles.begin(), triangles.end(), [&](aoeph::Triangle const& a_lhs, aoeph::Triangle const& a_rhs)
					{
						auto const cl = (a_lhs.p0 + a_lhs.p1 + a_lhs.p2) / 3.0f;
						auto const cr = (a_rhs.p0 + a_rhs.p1 + a_rhs.p2) / 3.0f;
						return glm::dot(cl, axis) < glm::dot(cr, axis);
					});

				return std::pair{
					std::span{ &triangles[0], triangles.size() /2 },
					std::span{ &triangles[triangles.size() / 2], triangles.size() - triangles.size() / 2 }
				};
			};

		auto const drawP = [&](std::span<aoeph::Triangle> triangles, glm::vec3 const& point, glm::vec3 const& axis, glm::vec3 color)
			{
				for (auto const& triangle : triangles)
				{
					auto const c = (triangle.p0 + triangle.p1 + triangle.p2) / 3.0f;
					auto const p = point + glm::dot(c - point, axis) * axis;
					debugMeshCtx.addLine(p, c, aoegl::toRgba(color));
				}
			};

		if (step-- == 0)
		{
			auto const carPos = (bounds.max + bounds.min) / 2.0f;
			auto const carBounds = aoeph::Aabb{ carPos - glm::vec3{2.0f, 0.0f, 1.0f}, carPos + glm::vec3{2.0f, 0.0f, 1.0f} };
			auto const carExtendedBounds = aoeph::Aabb{ carPos - glm::vec3{5.0f, 0.0f, 4.0f}, carPos + glm::vec3{5.0f, 0.0f, 4.0f} };

			auto const [intersectingTriangles, nonIntersectingTriangles] = intersectT(triangles, carExtendedBounds);
			drawT(intersectingTriangles, aoegl::k_white, true);
			auto const center = computeCenter(intersectingTriangles);
			debugMeshCtx.addSphere(center, 0.25f, aoegl::toRgba(glm::vec3{ 0.0f, 0.5f, 0.0f }));

			auto const dominantEigenVector = computeDominantEigenVector(intersectingTriangles, center);

			debugMeshCtx.addLine(center - 10.0f * dominantEigenVector, center + 10.0f * dominantEigenVector, aoegl::toRgba(glm::vec3{ 0.0f, 0.5f, 0.0f }));

			auto const [firstTriangles, secondTriangles] = bspSplit(intersectingTriangles, dominantEigenVector);
			drawP(firstTriangles, center, dominantEigenVector, aoegl::k_white);
			drawP(secondTriangles, center, dominantEigenVector, aoegl::k_white);
		}

		if (step-- == 0)
		{
			auto const carPos = (bounds.max + bounds.min) / 2.0f;
			auto const carBounds = aoeph::Aabb{ carPos - glm::vec3{2.0f, 0.0f, 1.0f}, carPos + glm::vec3{2.0f, 0.0f, 1.0f} };
			auto const carExtendedBounds = aoeph::Aabb{ carPos - glm::vec3{5.0f, 0.0f, 4.0f}, carPos + glm::vec3{5.0f, 0.0f, 4.0f} };

			auto const [intersectingTriangles, nonIntersectingTriangles] = intersectT(triangles, carExtendedBounds);
			auto const center = computeCenter(intersectingTriangles);
			debugMeshCtx.addSphere(center, 0.25f, aoegl::toRgba(glm::vec3{ 0.0f, 0.5f, 0.0f }));

			auto const dominantEigenVector = computeDominantEigenVector(intersectingTriangles, center);

			debugMeshCtx.addLine(center - 10.0f * dominantEigenVector, center + 10.0f * dominantEigenVector, aoegl::toRgba(glm::vec3{ 0.0f, 0.5f, 0.0f }));

			auto const [firstTriangles, secondTriangles] = bspSplit(intersectingTriangles, dominantEigenVector);
			drawT(firstTriangles, aoegl::k_red, true);
			drawP(firstTriangles, center, dominantEigenVector, glm::vec3{ 0.5f, 0.0f, 0.0f });
			drawT(secondTriangles, aoegl::k_blue, true);
			drawP(secondTriangles, center, dominantEigenVector, glm::vec3{ 0.0f, 0.0f, 0.5f });
		}

		if (step-- == 0)
		{
			auto const carPos = (bounds.max + bounds.min) / 2.0f;
			auto const carBounds = aoeph::Aabb{ carPos - glm::vec3{2.0f, 0.0f, 1.0f}, carPos + glm::vec3{2.0f, 0.0f, 1.0f} };
			auto const carExtendedBounds = aoeph::Aabb{ carPos - glm::vec3{5.0f, 0.0f, 4.0f}, carPos + glm::vec3{5.0f, 0.0f, 4.0f} };

			auto const [intersectingTriangles, nonIntersectingTriangles] = intersectT(triangles, carExtendedBounds);
			auto const center = computeCenter(intersectingTriangles);

			auto const dominantEigenVector = computeDominantEigenVector(intersectingTriangles, center);

			debugMeshCtx.addLine(center - 10.0f * dominantEigenVector, center + 10.0f * dominantEigenVector, aoegl::toRgba(glm::vec3{ 0.0f, 1.0f, 0.0f }));

			auto const [firstTriangles, secondTriangles] = bspSplit(intersectingTriangles, dominantEigenVector);
			drawT(firstTriangles, aoegl::k_red, true);
			drawT(secondTriangles, aoegl::k_blue, true);
		}

		auto const computeMinProjection = [&](std::span<aoeph::Triangle> triangles, glm::vec3 const& point, glm::vec3 const& axis)
			{
				auto min = std::numeric_limits<float>::infinity();
				for (auto const& triangle : triangles)
				{
					min = std::min(min, glm::dot(triangle.p0 - point, axis));
					min = std::min(min, glm::dot(triangle.p1 - point, axis));
					min = std::min(min, glm::dot(triangle.p2 - point, axis));
				}
				return min;
			};

		if (step-- == 0)
		{
			auto const carPos = (bounds.max + bounds.min) / 2.0f;
			auto const carBounds = aoeph::Aabb{ carPos - glm::vec3{2.0f, 0.0f, 1.0f}, carPos + glm::vec3{2.0f, 0.0f, 1.0f} };
			auto const carExtendedBounds = aoeph::Aabb{ carPos - glm::vec3{5.0f, 0.0f, 4.0f}, carPos + glm::vec3{5.0f, 0.0f, 4.0f} };

			auto const [intersectingTriangles, nonIntersectingTriangles] = intersectT(triangles, carExtendedBounds);
			auto const center = computeCenter(intersectingTriangles);

			auto const dominantEigenVector = computeDominantEigenVector(intersectingTriangles, center);

			auto const [firstTriangles, secondTriangles] = bspSplit(intersectingTriangles, dominantEigenVector);
			drawT(firstTriangles, glm::vec3{ 0.5f, 0.0f, 0.0f }, true);
			drawT(secondTriangles, glm::vec3{ 0.0f, 0.0f, 0.5f }, true);

			auto const planeLeft = glm::cross(dominantEigenVector, glm::vec3{ 0.0f, 1.0f, 0.0f });

			auto const firstPlanePoint = center - computeMinProjection(firstTriangles, center, -dominantEigenVector) * dominantEigenVector;
			debugMeshCtx.addLine(firstPlanePoint - 10.0f * planeLeft, firstPlanePoint + 10.0f * planeLeft, aoegl::k_red);
			debugMeshCtx.addLine(firstPlanePoint, firstPlanePoint - 1.0f * dominantEigenVector, aoegl::k_red);

			auto const secondPlanePoint = center + computeMinProjection(secondTriangles, center, dominantEigenVector) * dominantEigenVector;
			debugMeshCtx.addLine(secondPlanePoint - 10.0f * planeLeft, secondPlanePoint + 10.0f * planeLeft, aoegl::k_blue);
			debugMeshCtx.addLine(secondPlanePoint, secondPlanePoint + 1.0f * dominantEigenVector, aoegl::k_blue);
		}

		if (step-- == 0)
		{
			auto const carPos = (bounds.max + bounds.min) / 2.0f;
			auto const carBounds = aoeph::Aabb{ carPos - glm::vec3{2.0f, 0.0f, 1.0f}, carPos + glm::vec3{2.0f, 0.0f, 1.0f} };
			auto const carExtendedBounds = aoeph::Aabb{ carPos - glm::vec3{5.0f, 0.0f, 4.0f}, carPos + glm::vec3{5.0f, 0.0f, 4.0f} };

			auto const [intersectingTriangles, nonIntersectingTriangles] = intersectT(triangles, carExtendedBounds);
			auto const center = computeCenter(intersectingTriangles);

			auto const dominantEigenVector = computeDominantEigenVector(intersectingTriangles, center);

			auto const [firstTriangles, secondTriangles] = bspSplit(intersectingTriangles, dominantEigenVector);
			drawT(firstTriangles, aoegl::k_white, true);
			drawT(secondTriangles, glm::vec3{ 0.25f }, true);

			auto const planeLeft = glm::cross(dominantEigenVector, glm::vec3{ 0.0f, 1.0f, 0.0f });
			auto const firstPlanePoint = center - computeMinProjection(firstTriangles, center, -dominantEigenVector) * dominantEigenVector;
			debugMeshCtx.addLine(firstPlanePoint - 10.0f * planeLeft, firstPlanePoint + 10.0f * planeLeft, aoegl::k_white);
			debugMeshCtx.addLine(firstPlanePoint, firstPlanePoint - 1.0f * dominantEigenVector, aoegl::k_white);
		}

		if (step-- == 0)
		{
			auto const carPos = (bounds.max + bounds.min) / 2.0f;
			auto const carBounds = aoeph::Aabb{ carPos - glm::vec3{2.0f, 0.0f, 1.0f}, carPos + glm::vec3{2.0f, 0.0f, 1.0f} };
			auto const carExtendedBounds = aoeph::Aabb{ carPos - glm::vec3{5.0f, 0.0f, 4.0f}, carPos + glm::vec3{5.0f, 0.0f, 4.0f} };

			auto const [intersectingTriangles, nonIntersectingTriangles] = intersectT(triangles, carExtendedBounds);
			auto const center = computeCenter(intersectingTriangles);

			auto const dominantEigenVector = computeDominantEigenVector(intersectingTriangles, center);

			auto const [firstTriangles, secondTriangles] = bspSplit(intersectingTriangles, dominantEigenVector);
			drawT(firstTriangles, aoegl::k_white, true);
			drawT(secondTriangles, glm::vec3{ 0.25f }, true);

			auto const planeLeft = glm::cross(dominantEigenVector, glm::vec3{ 0.0f, 1.0f, 0.0f });
			auto const firstPlanePoint = center - computeMinProjection(firstTriangles, center, -dominantEigenVector) * dominantEigenVector;
			debugMeshCtx.addLine(firstPlanePoint - 10.0f * planeLeft, firstPlanePoint + 10.0f * planeLeft, aoegl::k_white);
			debugMeshCtx.addLine(firstPlanePoint, firstPlanePoint - 1.0f * dominantEigenVector, aoegl::k_white);

			auto const firstCenter = computeCenter(firstTriangles);
			debugMeshCtx.addSphere(firstCenter, 0.25f, aoegl::toRgba(glm::vec3{ 0.0f, 0.5f, 0.0f }));
			auto const firstDominantEigenVector = computeDominantEigenVector(firstTriangles, firstCenter);
			debugMeshCtx.addLine(firstCenter - 10.0f * firstDominantEigenVector, firstCenter + 10.0f * firstDominantEigenVector, aoegl::toRgba(glm::vec3{ 0.0f, 1.0f, 0.0f }));
		}

		if (step-- == 0)
		{
			auto const carPos = (bounds.max + bounds.min) / 2.0f;
			auto const carBounds = aoeph::Aabb{ carPos - glm::vec3{2.0f, 0.0f, 1.0f}, carPos + glm::vec3{2.0f, 0.0f, 1.0f} };
			auto const carExtendedBounds = aoeph::Aabb{ carPos - glm::vec3{5.0f, 0.0f, 4.0f}, carPos + glm::vec3{5.0f, 0.0f, 4.0f} };

			auto const [intersectingTriangles, nonIntersectingTriangles] = intersectT(triangles, carExtendedBounds);
			auto const center = computeCenter(intersectingTriangles);

			auto const dominantEigenVector = computeDominantEigenVector(intersectingTriangles, center);

			auto const [firstTriangles, secondTriangles] = bspSplit(intersectingTriangles, dominantEigenVector);
			drawT(firstTriangles, aoegl::k_white, true);
			drawT(secondTriangles, glm::vec3{ 0.25f }, true);

			auto const planeLeft = glm::cross(dominantEigenVector, glm::vec3{ 0.0f, 1.0f, 0.0f });
			auto const firstPlanePoint = center - computeMinProjection(firstTriangles, center, -dominantEigenVector) * dominantEigenVector;
			debugMeshCtx.addLine(firstPlanePoint - 10.0f * planeLeft, firstPlanePoint + 10.0f * planeLeft, aoegl::k_white);
			debugMeshCtx.addLine(firstPlanePoint, firstPlanePoint - 1.0f * dominantEigenVector, aoegl::k_white);

			auto const firstCenter = computeCenter(firstTriangles);
			debugMeshCtx.addSphere(firstCenter, 0.25f, aoegl::toRgba(glm::vec3{ 0.0f, 0.5f, 0.0f }));
			auto const firstDominantEigenVector = computeDominantEigenVector(firstTriangles, firstCenter);
			debugMeshCtx.addLine(firstCenter - 10.0f * firstDominantEigenVector, firstCenter + 10.0f * firstDominantEigenVector, aoegl::toRgba(glm::vec3{ 0.0f, 1.0f, 0.0f }));

			auto const [thirdTriangles, fourthTriangles] = bspSplit(firstTriangles, firstDominantEigenVector);
			drawP(thirdTriangles, firstCenter, firstDominantEigenVector, aoegl::k_white);
			drawP(fourthTriangles, firstCenter, firstDominantEigenVector, aoegl::k_white);
		}

		if (step-- == 0)
		{
			auto const carPos = (bounds.max + bounds.min) / 2.0f;
			auto const carBounds = aoeph::Aabb{ carPos - glm::vec3{2.0f, 0.0f, 1.0f}, carPos + glm::vec3{2.0f, 0.0f, 1.0f} };
			auto const carExtendedBounds = aoeph::Aabb{ carPos - glm::vec3{5.0f, 0.0f, 4.0f}, carPos + glm::vec3{5.0f, 0.0f, 4.0f} };

			auto const [intersectingTriangles, nonIntersectingTriangles] = intersectT(triangles, carExtendedBounds);
			auto const center = computeCenter(intersectingTriangles);

			auto const dominantEigenVector = computeDominantEigenVector(intersectingTriangles, center);

			auto const [firstTriangles, secondTriangles] = bspSplit(intersectingTriangles, dominantEigenVector);
			drawT(firstTriangles, aoegl::k_white, true);
			drawT(secondTriangles, glm::vec3{ 0.25f }, true);

			auto const planeLeft = glm::cross(dominantEigenVector, glm::vec3{ 0.0f, 1.0f, 0.0f });
			auto const firstPlanePoint = center - computeMinProjection(firstTriangles, center, -dominantEigenVector) * dominantEigenVector;
			debugMeshCtx.addLine(firstPlanePoint - 10.0f * planeLeft, firstPlanePoint + 10.0f * planeLeft, aoegl::k_white);
			debugMeshCtx.addLine(firstPlanePoint, firstPlanePoint - 1.0f * dominantEigenVector, aoegl::k_white);

			auto const firstCenter = computeCenter(firstTriangles);
			debugMeshCtx.addSphere(firstCenter, 0.25f, aoegl::toRgba(glm::vec3{ 0.0f, 0.5f, 0.0f }));
			auto const firstDominantEigenVector = computeDominantEigenVector(firstTriangles, firstCenter);
			debugMeshCtx.addLine(firstCenter - 10.0f * firstDominantEigenVector, firstCenter + 10.0f * firstDominantEigenVector, aoegl::toRgba(glm::vec3{ 0.0f, 1.0f, 0.0f }));

			auto const [thirdTriangles, fourthTriangles] = bspSplit(firstTriangles, firstDominantEigenVector);
			drawT(thirdTriangles, aoegl::k_red, true);
			drawP(thirdTriangles, firstCenter, firstDominantEigenVector, glm::vec3{ 0.5f, 0.0f, 0.0f });
			drawT(fourthTriangles, aoegl::k_blue, true);
			drawP(fourthTriangles, firstCenter, firstDominantEigenVector, glm::vec3{ 0.0f, 0.0f, 0.5f });
		}

		if (step-- == 0)
		{
			auto const carPos = (bounds.max + bounds.min) / 2.0f;
			auto const carBounds = aoeph::Aabb{ carPos - glm::vec3{2.0f, 0.0f, 1.0f}, carPos + glm::vec3{2.0f, 0.0f, 1.0f} };
			auto const carExtendedBounds = aoeph::Aabb{ carPos - glm::vec3{5.0f, 0.0f, 4.0f}, carPos + glm::vec3{5.0f, 0.0f, 4.0f} };

			auto const [intersectingTriangles, nonIntersectingTriangles] = intersectT(triangles, carExtendedBounds);
			auto const center = computeCenter(intersectingTriangles);

			auto const dominantEigenVector = computeDominantEigenVector(intersectingTriangles, center);

			auto const [firstTriangles, secondTriangles] = bspSplit(intersectingTriangles, dominantEigenVector);
			drawT(firstTriangles, aoegl::k_white, true);
			drawT(secondTriangles, glm::vec3{ 0.25f }, true);

			auto const planeLeft = glm::cross(dominantEigenVector, glm::vec3{ 0.0f, 1.0f, 0.0f });
			auto const firstPlanePoint = center - computeMinProjection(firstTriangles, center, -dominantEigenVector) * dominantEigenVector;
			debugMeshCtx.addLine(firstPlanePoint - 10.0f * planeLeft, firstPlanePoint + 10.0f * planeLeft, aoegl::k_white);
			debugMeshCtx.addLine(firstPlanePoint, firstPlanePoint - 1.0f * dominantEigenVector, aoegl::k_white);

			auto const firstCenter = computeCenter(firstTriangles);
			auto const firstDominantEigenVector = computeDominantEigenVector(firstTriangles, firstCenter);
			debugMeshCtx.addLine(firstCenter - 10.0f * firstDominantEigenVector, firstCenter + 10.0f * firstDominantEigenVector, aoegl::toRgba(glm::vec3{ 0.0f, 1.0f, 0.0f }));

			auto const [thirdTriangles, fourthTriangles] = bspSplit(firstTriangles, firstDominantEigenVector);
			drawT(thirdTriangles, aoegl::k_red, true);
			drawT(fourthTriangles, aoegl::k_blue, true);
		}

		if (step-- == 0)
		{
			auto const carPos = (bounds.max + bounds.min) / 2.0f;
			auto const carBounds = aoeph::Aabb{ carPos - glm::vec3{2.0f, 0.0f, 1.0f}, carPos + glm::vec3{2.0f, 0.0f, 1.0f} };
			auto const carExtendedBounds = aoeph::Aabb{ carPos - glm::vec3{5.0f, 0.0f, 4.0f}, carPos + glm::vec3{5.0f, 0.0f, 4.0f} };

			auto const [intersectingTriangles, nonIntersectingTriangles] = intersectT(triangles, carExtendedBounds);
			auto const center = computeCenter(intersectingTriangles);

			auto const dominantEigenVector = computeDominantEigenVector(intersectingTriangles, center);

			auto const [firstTriangles, secondTriangles] = bspSplit(intersectingTriangles, dominantEigenVector);
			drawT(firstTriangles, aoegl::k_white, true);
			drawT(secondTriangles, glm::vec3{ 0.25f }, true);

			auto const planeLeft = glm::cross(dominantEigenVector, glm::vec3{ 0.0f, 1.0f, 0.0f });
			auto const firstPlanePoint = center - computeMinProjection(firstTriangles, center, -dominantEigenVector) * dominantEigenVector;
			debugMeshCtx.addLine(firstPlanePoint - 10.0f * planeLeft, firstPlanePoint + 10.0f * planeLeft, aoegl::k_white);
			debugMeshCtx.addLine(firstPlanePoint, firstPlanePoint - 1.0f * dominantEigenVector, aoegl::k_white);

			auto const firstCenter = computeCenter(firstTriangles);
			auto const firstDominantEigenVector = computeDominantEigenVector(firstTriangles, firstCenter);

			auto const [thirdTriangles, fourthTriangles] = bspSplit(firstTriangles, firstDominantEigenVector);
			drawT(thirdTriangles, aoegl::k_red, true);
			drawT(fourthTriangles, aoegl::k_blue, true);


			auto const firstPlaneLeft = glm::cross(firstDominantEigenVector, glm::vec3{ 0.0f, 1.0f, 0.0f });

			auto const thirdPlanePoint = firstCenter - computeMinProjection(thirdTriangles, firstCenter, -firstDominantEigenVector) * firstDominantEigenVector;
			debugMeshCtx.addLine(thirdPlanePoint - 10.0f * firstPlaneLeft, thirdPlanePoint + 10.0f * firstPlaneLeft, aoegl::k_red);
			debugMeshCtx.addLine(thirdPlanePoint, thirdPlanePoint - 1.0f * firstDominantEigenVector, aoegl::k_red);

			auto const fourthPlanePoint = firstCenter + computeMinProjection(secondTriangles, firstCenter, firstDominantEigenVector) * firstDominantEigenVector;
			debugMeshCtx.addLine(fourthPlanePoint - 10.0f * firstPlaneLeft, fourthPlanePoint + 10.0f * firstPlaneLeft, aoegl::k_blue);
			debugMeshCtx.addLine(fourthPlanePoint, fourthPlanePoint + 1.0f * firstDominantEigenVector, aoegl::k_blue);
		}
	}
}
