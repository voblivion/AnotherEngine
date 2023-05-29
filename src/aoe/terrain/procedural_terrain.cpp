#include <vob/aoe/terrain/procedural_terrain.h>

#include <vob/misc/random/simplex.h>

#include <span>

#include <vob/aoe/pathfinding/a_star.h>


namespace vob::aoetr
{
	aoegl::mesh_data generate_procedural_terrain(std::int32_t a_size, float a_cellSize, float a_height, float a_f)
	{
		layer uniqueLayer{ a_height, a_f };
		return generate_procedural_terrain(a_cellSize * a_size, a_cellSize, std::span{ &uniqueLayer, 1 }, true);
	}

	aoegl::mesh_data generate_procedural_terrain(
		float a_size, float a_cellSize, std::span<layer> a_layers, bool a_useSmoothShading)
	{
		auto const subdivisions = static_cast<std::int32_t>(std::ceil(a_size / a_cellSize));
		auto const vertexCount = (subdivisions + 1) * (subdivisions + 1);

		std::vector<std::vector<glm::vec3>> pos;
		pos.reserve(subdivisions + 3);
		for (int i = -1; i <= subdivisions + 2; ++i)
		{
			auto& line = pos.emplace_back();
			line.reserve(subdivisions + 3);
			for (int j = -1; j <= subdivisions + 2; ++j)
			{
				auto const x = float(i) * a_cellSize - a_size / 2;
				auto const y = float(j) * a_cellSize - a_size / 2;

				auto height = 0.0;
				for (auto const& layer : a_layers)
				{
					height += layer.m_height * misrn::simplex::noise(
						x * layer.m_frequency + layer.m_offset.x,
						y * layer.m_frequency + layer.m_offset.y);
				}

				line.emplace_back(x, height, y);
			}
		}

		aoegl::mesh_data mesh;
		mesh.m_positions.reserve(vertexCount);
		mesh.m_textureCoords.reserve(vertexCount);
		mesh.m_normals.reserve(vertexCount);
		mesh.m_tangents.reserve(vertexCount);
		mesh.m_triangles.reserve(subdivisions * subdivisions * 2);

		auto const f = [](auto const p0, auto const p1, auto const p2)
		{
			auto faceNormal = glm::cross(p1 - p0, p2 - p0);
			return glm::normalize(faceNormal);
		};

		auto const n = [&f, &pos](auto i, auto j)
		{
			auto normal = glm::vec3{ 0.0f };
			normal += f(pos[i - 1][j - 1], pos[i - 1][j], pos[i][j]);
			normal += f(pos[i - 1][j - 1], pos[i][j], pos[i][j - 1]);
			normal += f(pos[i][j - 1], pos[i][j], pos[i + 1][j - 1]);
			normal += f(pos[i + 1][j - 1], pos[i + 1][j], pos[i][j]);
			normal += f(pos[i][j], pos[i + 1][j + 1], pos[i + 1][j]);
			normal += f(pos[i][j], pos[i][j + 1], pos[i + 1][j + 1]);
			normal += f(pos[i][j], pos[i - 1][j + 1], pos[i][j + 1]);
			normal += f(pos[i][j], pos[i - 1][j], pos[i - 1][j + 1]);

			auto averageNormal = glm::normalize(normal);
			return averageNormal;
		};

		for (auto i = 1; i <= subdivisions; ++i)
		{
			for (auto j = 1; j <= subdivisions; ++j)
			{
				glm::vec3 p0 = pos[i][j];
				glm::vec3 p1 = pos[i][j + 1];
				glm::vec3 p2 = pos[i + 1][j];
				glm::vec3 p3 = pos[i + 1][j + 1];

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