#include <aoe/common/render/AssimpLoader.h>

#include <aoe/core/standard/IgnorableAssert.h>
#include <assimp/postprocess.h>
#include "aoe/core/data/LoaderUtils.h"

namespace aoe
{
	namespace common
	{
		std::pmr::vector<Vertex> extractVertices(
			std::pmr::memory_resource* a_resource, aiMesh const& a_meshData)
		{
			std::pmr::vector<Vertex> t_vertices{
				sta::Allocator<Vertex>{ a_resource } };
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

		Mesh extractMesh(std::pmr::memory_resource* a_resource
			, aiMesh const& a_meshData)
		{
			auto t_vertices = extractVertices(a_resource, a_meshData);

			// Load faces, only triangles supported
			std::pmr::vector<std::uint32_t> t_faces{
				sta::Allocator<std::uint32_t>{ a_resource } };
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

		Model extractModel(std::pmr::memory_resource* a_resource
			, aiScene const& a_sceneData)
		{
			std::pmr::vector<Mesh> t_meshes{ sta::Allocator<Mesh>{ a_resource} };
			t_meshes.reserve(a_sceneData.mNumMeshes);
			for (auto k = 0u; k < a_sceneData.mNumMeshes; ++k)
			{
				t_meshes.emplace_back(extractMesh(a_resource
					, *a_sceneData.mMeshes[k]));
			}

			return Model{ std::move(t_meshes) };
		}

		AssimpLoader::AssimpLoader(std::pmr::memory_resource* a_resource)
			: m_resource{ a_resource }
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

			return sta::allocatePolymorphicWith<Model>(m_resource
				, extractModel(m_resource, *scene));
		}
	}
}
