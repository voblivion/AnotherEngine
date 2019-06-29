#include <aoe/common/render/AssimpLoader.h>

#include <aoe/core/standard/IgnorableAssert.h>
#include <assimp/postprocess.h>
#include "aoe/core/data/LoaderUtils.h"

namespace aoe
{
	namespace common
	{
		std::pmr::vector<Vertex> extractVertices(
			sta::Allocator<std::byte>& a_allocator, aiMesh const& a_meshData)
		{
			std::pmr::vector<Vertex> t_vertices{ a_allocator };
			t_vertices.reserve(a_meshData.mNumVertices);
			if (a_meshData.mTextureCoords[0] != nullptr)
			{
				for (auto k = 0u; k < a_meshData.mNumVertices; ++k)
				{
					t_vertices.emplace_back(
						toGlmVec3(a_meshData.mVertices[k])
						, toGlmVec3(a_meshData.mNormals[k])
						, toGlmVec2(a_meshData.mTextureCoords[0][k])
					);
				}
			}
			else
			{
				for (auto k = 0u; k < a_meshData.mNumVertices; ++k)
				{
					t_vertices.emplace_back(
						toGlmVec3(a_meshData.mVertices[k])
						, toGlmVec3(a_meshData.mNormals[k])
						, glm::vec2{ 0, 0 }
					);
				}
			}

			return t_vertices;
		}

		Mesh extractMesh(sta::Allocator<std::byte>& a_allocator
			, aiMesh const& a_meshData)
		{
			auto t_vertices = extractVertices(a_allocator, a_meshData);

			// Load faces, only triangles supported
			std::pmr::vector<std::uint32_t> t_faces{ a_allocator };
			t_faces.reserve(a_meshData.mNumFaces * 3u);
			for (auto k = 0u; k < a_meshData.mNumFaces; ++k)
			{
				if (a_meshData.mFaces->mNumIndices == 3)
				{
					t_faces.emplace_back(a_meshData.mFaces[k].mIndices[0]);
					t_faces.emplace_back(a_meshData.mFaces[k].mIndices[1]);
					t_faces.emplace_back(a_meshData.mFaces[k].mIndices[2]);
				}
			}

			return Mesh{ std::move(t_vertices), std::move(t_faces) };
		}

		Model extractModel(sta::Allocator<std::byte>& a_allocator
			, aiScene const& a_sceneData)
		{
			std::pmr::vector<Mesh> t_meshes{ a_allocator };
			t_meshes.reserve(a_sceneData.mNumMeshes);
			for (auto k = 0u; k < a_sceneData.mNumMeshes; ++k)
			{
				t_meshes.emplace_back(extractMesh(a_allocator
					, *a_sceneData.mMeshes[k]));
			}

			return Model{ std::move(t_meshes) };
		}

		AssimpLoader::AssimpLoader(sta::Allocator<std::byte> const& a_allocator)
			: m_allocator{ a_allocator }
		{}

		std::shared_ptr<sta::ADynamicType> AssimpLoader::load(
			std::istream& a_inputStream)
		{
			// Load stream in memory
			auto t_data = data::getData(a_inputStream);
			ignorableAssert(!t_data.empty());

			// Make Assimp load scene
			auto const scene = m_importer.ReadFileFromMemory(t_data.data()
				, t_data.size(), aiProcess_Triangulate | aiProcess_FlipUVs);

			ignorableAssert(scene != nullptr
				&& !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
				if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
				{
					return nullptr;
				}

			return sta::allocatePolymorphic<Model>(m_allocator
				, extractModel(m_allocator, *scene));
		}
	}
}
