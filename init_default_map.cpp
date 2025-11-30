
#include <vob/aoe/engine/world.h>
#include <vob/aoe/spacetime/transform.h>
#include <vob/aoe/spacetime/soft_follow.h>

#include "DataHolder.h"

/*std::vector<vob::aoeph::triangle> transform_surface(glm::vec3 const& a_position, glm::quat const& a_rotation, std::vector<vob::aoeph::triangle> const& a_triangles)
{
	std::vector<vob::aoeph::triangle> result;
	for (auto& tr : a_triangles)
	{
		result.emplace_back(
			a_position + a_rotation * tr.p0,
			a_position + a_rotation * tr.p1,
			a_position + a_rotation * tr.p2);
	}
	return result;
}

std::vector<vob::aoeph::triangle> create_flat_surface(glm::vec3 const& a_position, glm::quat const& a_rotation)
{
	std::vector<vob::aoeph::triangle> triangles;
	triangles.emplace_back(
		glm::vec3{ 32.0f, 2.0f, 0.0f },
		glm::vec3{ 0.0f, 2.0f, 0.0f },
		glm::vec3{ 0.0f, 2.0f, 32.0f }
	);
	triangles.emplace_back(
		glm::vec3{ 32.0f, 2.0f, 32.0f },
		glm::vec3{ 32.0f, 2.0f, 0.0f },
		glm::vec3{ 0.0f, 2.0f, 32.0f }
	);
	triangles.emplace_back(
		glm::vec3{0.0f, 2.0f, 0.0f},
		glm::vec3{0.0f, 0.0f, 0.0f},
		glm::vec3{0.0f, 2.0f, 32.0f}
	);
	triangles.emplace_back(
		glm::vec3{ 0.0f, 2.0f, 0.0f },
		glm::vec3{ 0.0f, 0.0f, 0.0f },
		glm::vec3{ 0.0f, 2.0f, 32.0f }
	);
	triangles.emplace_back(
		glm::vec3{ 0.0f, 0.0f, 0.0f },
		glm::vec3{ 0.0f, 0.0f, 32.0f },
		glm::vec3{ 0.0f, 2.0f, 32.0f }
	);
	triangles.emplace_back(
		glm::vec3{ 32.0f, 2.0f, 0.0f },
		glm::vec3{ 32.0f, 2.0f, 32.0f },
		glm::vec3{ 32.0f, 0.0f, 0.0f }
	);
	triangles.emplace_back(
		glm::vec3{ 32.0f, 2.0f, 32.0f },
		glm::vec3{ 32.0f, 0.0f, 32.0f },
		glm::vec3{ 32.0f, 0.0f, 0.0f }
	);
	return transform_surface(a_position, a_rotation, triangles);
}

std::vector<vob::aoeph::triangle> create_slope_surface(glm::vec3 const& a_position, glm::quat const& a_rotation)
{
	std::vector<vob::aoeph::triangle> triangles;
	triangles.emplace_back(
		glm::vec3{ 32.0f, 2.0f, 0.0f },
		glm::vec3{ 0.0f, 2.0f, 0.0f },
		glm::vec3{ 0.0f, 18.0f, 32.0f }
	);
	triangles.emplace_back(
		glm::vec3{ 32.0f, 18.0f, 32.0f },
		glm::vec3{ 32.0f, 2.0f, 0.0f },
		glm::vec3{ 0.0f, 18.0f, 32.0f }
	);
	return transform_surface(a_position, a_rotation, triangles);
}

std::pair<float, float> bezier_cubic(float y0, float x1, float y1, float x2, float y2, float x3, float y3, float t)
{
	auto const u = (1.0f - t);
	auto const x = u * u * u * 0.f + 3.0f * u * u * t * x1 + 3.0f * u * t * t * x2 + t * t * t * x3;
	auto const y = u * u * u * y0 + 3.0f * u * u * t * y1 + 3.0f * u * t * t * y2 + t * t * t * y3;
	return std::make_pair(x, y);
}

std::vector<vob::aoeph::triangle> create_smooth_step_surface(glm::vec3 const& a_position, glm::quat const& a_rotation)
{
	constexpr int32_t k_subdivisions = 32;

	auto p = [](int32_t s, int32_t subdivisions) {
		return bezier_cubic(0.0f, 16.0f, 0.0f, 16.0f, 8.0f, 32.0f, 8.0f, static_cast<float>(s) / k_subdivisions);
	};

	std::vector<vob::aoeph::triangle> triangles;
	for (int32_t s = 0; s < k_subdivisions; ++s)
	{
		auto const [x0, y0] = p(s, k_subdivisions);
		auto const [x1, y1] = p(s + 1, k_subdivisions);
		triangles.emplace_back(
			glm::vec3{ 32.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y1, x1 }
		);
		triangles.emplace_back(
			glm::vec3{ 32.0f, 2.0f + y1, x1 },
			glm::vec3{ 32.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y1, x1 }
		);
	}
	return transform_surface(a_position, a_rotation, triangles);
}

std::vector<vob::aoeph::triangle> create_smooth_step2_surface(glm::vec3 const& a_position, glm::quat const& a_rotation)
{
	constexpr int32_t k_subdivisions = 32;

	auto p = [](int32_t s, int32_t subdivisions) {
		return bezier_cubic(0.0f, 16.0f, 0.0f, 16.0f, 16.0f, 32.0f, 16.0f, static_cast<float>(s) / k_subdivisions);
		};

	std::vector<vob::aoeph::triangle> triangles;
	for (int32_t s = 0; s < k_subdivisions; ++s)
	{
		auto const [x0, y0] = p(s, k_subdivisions);
		auto const [x1, y1] = p(s + 1, k_subdivisions);
		triangles.emplace_back(
			glm::vec3{ 32.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y1, x1 }
		);
		triangles.emplace_back(
			glm::vec3{ 32.0f, 2.0f + y1, x1 },
			glm::vec3{ 32.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y1, x1 }
		);
	}
	return transform_surface(a_position, a_rotation, triangles);
}

std::vector<vob::aoeph::triangle> create_smooth_start_surface(glm::vec3 const& a_position, glm::quat const& a_rotation)
{
	constexpr int32_t k_subdivisions = 32;

	auto p = [](int32_t s, int32_t subdivisions) {
		return bezier_cubic(0.0f, 16.0f, 0.0f, 16.0f, 0.0f, 32.0f, 8.0f, static_cast<float>(s) / k_subdivisions);
		};

	std::vector<vob::aoeph::triangle> triangles;
	for (int32_t s = 0; s < k_subdivisions; ++s)
	{
		auto const [x0, y0] = p(s, k_subdivisions);
		auto const [x1, y1] = p(s + 1, k_subdivisions);
		triangles.emplace_back(
			glm::vec3{ 32.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y1, x1 }
		);
		triangles.emplace_back(
			glm::vec3{ 32.0f, 2.0f + y1, x1 },
			glm::vec3{ 32.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y1, x1 }
		);
	}
	return transform_surface(a_position, a_rotation, triangles);
}

std::vector<vob::aoeph::triangle> create_smooth_stop_surface(glm::vec3 const& a_position, glm::quat const& a_rotation)
{
	constexpr int32_t k_subdivisions = 32;

	auto p = [](int32_t s, int32_t subdivisions) {
		return bezier_cubic(0.0f, 16.0f, 8.0f, 16.0f, 8.0f, 32.0f, 8.0f, static_cast<float>(s) / k_subdivisions);
		};

	std::vector<vob::aoeph::triangle> triangles;
	for (int32_t s = 0; s < k_subdivisions; ++s)
	{
		auto const [x0, y0] = p(s, k_subdivisions);
		auto const [x1, y1] = p(s + 1, k_subdivisions);
		triangles.emplace_back(
			glm::vec3{ 32.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y1, x1 }
		);
		triangles.emplace_back(
			glm::vec3{ 32.0f, 2.0f + y1, x1 },
			glm::vec3{ 32.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y1, x1 }
		);
	}
	return transform_surface(a_position, a_rotation, triangles);
}

std::vector<vob::aoeph::triangle> _create_turn_surface(glm::vec3 const& a_position, glm::quat const& a_rotation, int32_t a_size)
{
	constexpr int32_t k_subdivisions = 32;

	auto const minR = 32.0f * a_size;
	auto const maxR = 32.0f * (a_size + 1);

	std::vector<vob::aoeph::triangle> triangles;
	for (int32_t s = 0; s < k_subdivisions; ++s)
	{
		auto const r0 = 3.141592f / 2.0f * static_cast<float>(s) / k_subdivisions;
		auto const r1 = 3.141592f / 2.0f * static_cast<float>(s + 1) / k_subdivisions;
		triangles.emplace_back(
			glm::vec3{ maxR - minR * std::cos(r0), 2.0f, minR * std::sin(r0) },
			glm::vec3{ maxR - maxR * std::cos(r0), 2.0f, maxR * std::sin(r0) },
			glm::vec3{ maxR - maxR * std::cos(r1), 2.0f, maxR * std::sin(r1) }
		);
		triangles.emplace_back(
			glm::vec3{ maxR - minR * std::cos(r1), 2.0f, minR * std::sin(r1) },
			glm::vec3{ maxR - minR * std::cos(r0), 2.0f, minR * std::sin(r0) },
			glm::vec3{ maxR - maxR * std::cos(r1), 2.0f, maxR * std::sin(r1) }
		);
		triangles.emplace_back(
			glm::vec3{ maxR - maxR * std::cos(r0), 2.0f, maxR * std::sin(r0) },
			glm::vec3{ maxR - maxR * std::cos(r0), 0.0f, maxR * std::sin(r0) },
			glm::vec3{ maxR - maxR * std::cos(r1), 2.0f, maxR * std::sin(r1) });
		triangles.emplace_back(
			glm::vec3{ maxR - maxR * std::cos(r0), 0.0f, maxR * std::sin(r0) },
			glm::vec3{ maxR - maxR * std::cos(r1), 0.0f, maxR * std::sin(r1) },
			glm::vec3{ maxR - maxR * std::cos(r1), 2.0f, maxR * std::sin(r1) });
		triangles.emplace_back(
			glm::vec3{ maxR - minR * std::cos(r1), 2.0f, minR * std::sin(r1) },
			glm::vec3{ maxR - minR * std::cos(r1), 0.0f, minR * std::sin(r1) },
			glm::vec3{ maxR - minR * std::cos(r0), 2.0f, minR * std::sin(r0) }
		);
		triangles.emplace_back(
			glm::vec3{ maxR - minR * std::cos(r1), 0.0f, minR * std::sin(r1) },
			glm::vec3{ maxR - minR * std::cos(r0), 0.0f, minR * std::sin(r0) },
			glm::vec3{ maxR - minR * std::cos(r0), 2.0f, minR * std::sin(r0) }
		);
	}
	return transform_surface(a_position, a_rotation, triangles);
}

std::vector<vob::aoeph::triangle> create_turn0_surface(glm::vec3 const& a_position, glm::quat const& a_rotation)
{
	return _create_turn_surface(a_position, a_rotation, 0);
}

std::vector<vob::aoeph::triangle> create_turn_surface(glm::vec3 const& a_position, glm::quat const& a_rotation)
{
	return _create_turn_surface(a_position, a_rotation, 1);
}

std::vector<vob::aoeph::triangle> create_turn2_surface(glm::vec3 const& a_position, glm::quat const& a_rotation)
{
	return _create_turn_surface(a_position, a_rotation, 2);
}*/
