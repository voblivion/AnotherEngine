#include <vob/aoe/terrain/procedural_terrain.h>

#include <vob/aoe/pathfinding/a_star.h>

#include <vob/misc/random/simplex.h>
#include <vob/misc/std/vector2d.h>

#include <bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#include <span>


namespace
{
	float calculate_height(glm::vec2 const& a_pos2d, std::span<vob::aoetr::layer> a_layers)
	{
		auto height = 0.0f;
		for (auto const& layer : a_layers)
		{
			height += static_cast<float>(layer.m_height * vob::misrn::simplex::noise(
				a_pos2d.x * layer.m_frequency + layer.m_offset.x,
				a_pos2d.y * layer.m_frequency + layer.m_offset.y));
		}
		return height;
	}

	template <typename TProcessor>
	vob::mistd::vector2d<decltype(std::declval<TProcessor>()(std::declval<glm::vec3 const&>()))> generate_procedural(
		glm::vec2 a_center,
		glm::vec2 a_size,
		glm::ivec2 a_subdivisions,
		std::span<vob::aoetr::layer> a_layers,
		TProcessor a_processor)
	{
		using type = decltype(std::declval<TProcessor>()(std::declval<glm::vec3 const&>()));

		auto const cellSize = glm::vec2{ a_size.x / a_subdivisions.x, a_size.y / a_subdivisions.y };
		auto const start = a_center - a_size / 2.0f;

		vob::mistd::vector2d<type> result{ vob::mistd::size2d{a_subdivisions.x + 1, a_subdivisions.y + 1} };
		for (auto const& index : result.keys())
		{
			auto const pos2d = start + glm::vec2{ index.x * cellSize.x, index.y * cellSize.y };
			result[index] = a_processor(glm::vec3{ pos2d.x, calculate_height(pos2d, a_layers), pos2d.y });
		}

		return result;
	}
}


namespace vob::aoetr
{
	mistd::vector2d<float> generate_procedural_heights(
		glm::vec2 a_center, glm::vec2 a_size, glm::ivec2 a_subdivisions, std::span<layer> a_layers)
	{
		return generate_procedural(
			a_center, a_size, a_subdivisions, a_layers, [](glm::vec3 const& a_pos) { return a_pos.y; });
	}

	mistd::vector2d<glm::vec3> generated_procedural_positions(
		glm::vec2 a_center, glm::vec2 a_size, glm::ivec2 a_subdivisions, std::span<layer> a_layers)
	{
		return generate_procedural(
			a_center, a_size, a_subdivisions, a_layers, [](glm::vec3 const& a_pos) { return a_pos; });
	}

	aoegl::mesh_data generate_procedural_mesh(
		glm::vec2 a_center, glm::vec2 a_size, glm::ivec2 a_subdivisions, std::span<layer> a_layers, bool a_useSmoothShading)
	{

		auto const cellSize = glm::vec2{ a_size.x / a_subdivisions.x, a_size.y / a_subdivisions.y };
		auto const vertexCount = (a_subdivisions.x + 2) * (a_subdivisions.y + 2);

		auto pos = generated_procedural_positions(a_center, a_size + 2.0f * cellSize, a_subdivisions + glm::ivec2{2, 2}, a_layers);

		aoegl::mesh_data mesh;
		mesh.m_positions.reserve(vertexCount);
		mesh.m_textureCoords.reserve(vertexCount);
		mesh.m_normals.reserve(vertexCount);
		mesh.m_tangents.reserve(vertexCount);
		mesh.m_triangles.reserve(a_subdivisions.x * a_subdivisions.y * 2);

		auto const f = [](auto const p0, auto const p1, auto const p2)
		{
			auto faceNormal = glm::cross(p1 - p0, p2 - p0);
			return glm::normalize(faceNormal);
		};

		auto const n = [&f, &pos](auto i, auto j)
		{
			auto normal = glm::vec3{ 0.0f };
			normal += f(pos[{i - 1, j - 1}], pos[{i - 1, j}], pos[{i, j}]);
			normal += f(pos[{i - 1, j - 1}], pos[{i, j}], pos[{i, j - 1}]);
			normal += f(pos[{i, j - 1}], pos[{i, j}], pos[{i + 1, j - 1}]);
			normal += f(pos[{i + 1, j - 1}], pos[{i + 1, j}], pos[{i, j}]);
			normal += f(pos[{i, j}], pos[{i + 1, j + 1}], pos[{i + 1, j}]);
			normal += f(pos[{i, j}], pos[{i, j + 1}], pos[{i + 1, j + 1}]);
			normal += f(pos[{i, j}], pos[{i - 1, j + 1}], pos[{i, j + 1}]);
			normal += f(pos[{i, j}], pos[{i - 1, j}], pos[{i - 1, j + 1}]);

			auto averageNormal = glm::normalize(normal);
			return averageNormal;
		};

		for (auto i = 1; i <= a_subdivisions.x; ++i)
		{
			for (auto j = 1; j <= a_subdivisions.y; ++j)
			{
				glm::vec3 p0 = pos[{i, j}];
				glm::vec3 p1 = pos[{i, j + 1}];
				glm::vec3 p2 = pos[{i + 1, j}];
				glm::vec3 p3 = pos[{i + 1, j + 1}];

				glm::vec2 tc0 = glm::vec2{ 0.0f, 0.0f };
				glm::vec2 tc1 = glm::vec2{ 0.0f, 1.0f };
				glm::vec2 tc2 = glm::vec2{ 1.0f, 0.0f };
				glm::vec2 tc3 = glm::vec2{ 1.0f, 1.0f };

				if ((i + j) % 2 == 0)
				{
					auto const tmp = p0;
					p0 = p1;
					p1 = p3;
					p3 = p2;
					p2 = tmp;
				}

				auto t0 = glm::vec3{ 1.0, 0.0, 0.0 };

				if (a_useSmoothShading)
				{
					auto n0 = n(i, j);
					auto n1 = n(i, j+1);
					auto n2 = n(i+1, j);
					auto n3 = n(i+1, j+1);

					if ((i + j) % 2 == 0)
					{
						auto const tmp = n0;
						n0 = n1;
						n1 = n3;
						n3 = n2;
						n2 = tmp;
					}
					auto const k = static_cast<std::uint32_t>(mesh.m_positions.size());

					mesh.m_positions.emplace_back(p0);
					mesh.m_positions.emplace_back(p1);
					mesh.m_positions.emplace_back(p2);
					mesh.m_positions.emplace_back(p3);

					mesh.m_textureCoords.emplace_back(tc0);
					mesh.m_textureCoords.emplace_back(tc1);
					mesh.m_textureCoords.emplace_back(tc2);
					mesh.m_textureCoords.emplace_back(tc3);

					mesh.m_normals.emplace_back(n0);
					mesh.m_normals.emplace_back(n1);
					mesh.m_normals.emplace_back(n2);
					mesh.m_normals.emplace_back(n3);

					mesh.m_tangents.emplace_back(t0);
					mesh.m_tangents.emplace_back(t0);
					mesh.m_tangents.emplace_back(t0);
					mesh.m_tangents.emplace_back(t0);

					mesh.m_triangles.emplace_back(k + 0, k + 1, k + 2);
					mesh.m_triangles.emplace_back(k + 3, k + 2, k + 1);
				}
				else
				{
					auto n0 = f(p0, p1, p2);
					auto n1 = f(p3, p2, p1);

					auto const k = static_cast<std::uint32_t>(mesh.m_positions.size());
					mesh.m_positions.emplace_back(p0);
					mesh.m_positions.emplace_back(p1);
					mesh.m_positions.emplace_back(p2);
					mesh.m_positions.emplace_back(p3);
					mesh.m_positions.emplace_back(p2);
					mesh.m_positions.emplace_back(p1);

					mesh.m_textureCoords.emplace_back(tc0);
					mesh.m_textureCoords.emplace_back(tc1);
					mesh.m_textureCoords.emplace_back(tc2);
					mesh.m_textureCoords.emplace_back(tc3);
					mesh.m_textureCoords.emplace_back(tc2);
					mesh.m_textureCoords.emplace_back(tc1);

					mesh.m_normals.emplace_back(n0);
					mesh.m_normals.emplace_back(n0);
					mesh.m_normals.emplace_back(n0);
					mesh.m_normals.emplace_back(n1);
					mesh.m_normals.emplace_back(n1);
					mesh.m_normals.emplace_back(n1);

					mesh.m_tangents.emplace_back(t0);
					mesh.m_tangents.emplace_back(t0);
					mesh.m_tangents.emplace_back(t0);
					mesh.m_tangents.emplace_back(t0);
					mesh.m_tangents.emplace_back(t0);
					mesh.m_tangents.emplace_back(t0);

					mesh.m_triangles.emplace_back(k + 0, k + 1, k + 2);
					mesh.m_triangles.emplace_back(k + 3, k + 4, k + 5);
				}
			}
		}

		return mesh;
	}
}