#include <vob/aoe/common/render/AssimpLoader.h>

#include <vob/sta/ignorable_assert.h>
#include <assimp/postprocess.h>
#include <vob/aoe/core/data/LoaderUtils.h>
#include <assimp/scene.h>
#include <vob/aoe/common/opengl/Handle.h>
#include <vob/aoe/common/render/AssimpUtils.h>

namespace vob::aoe::ogl
{
	std::pmr::vector<Vertex> extractVertices(
		std::pmr::memory_resource* a_resource
		, aiMesh const& a_meshData
	)
	{
		using allocator = std::pmr::polymorphic_allocator<Vertex>;
		std::pmr::vector<Vertex> t_vertices{ allocator{ a_resource } };

		t_vertices.reserve(a_meshData.mNumVertices);
		if (a_meshData.mTextureCoords[0] != nullptr)
		{
			for (auto k = 0u; k < a_meshData.mNumVertices; ++k)
			{
				Vertex t{
					toGlmVec3(a_meshData.mVertices[k])
					, toGlmVec3(a_meshData.mNormals[k])
					, toGlmVec2(a_meshData.mTextureCoords[0][k])
				};
				t_vertices.emplace_back(t);
			}
		}
		else
		{
			for (auto k = 0u; k < a_meshData.mNumVertices; ++k)
			{
				Vertex t{ toGlmVec3(a_meshData.mVertices[k])
					, toGlmVec3(a_meshData.mNormals[k])
					, glm::vec2{ 0, 0 }
				};
				t_vertices.emplace_back(t);
			}
		}

		return t_vertices;
	}

	StaticMesh extractMesh(std::pmr::memory_resource* a_resource
		, aiMesh const& a_meshData)
	{
		auto t_vertices = extractVertices(a_resource, a_meshData);

		// Load faces, only triangles supported
		std::pmr::vector<std::uint32_t> t_faces{
			std::pmr::polymorphic_allocator<std::uint32_t>{ a_resource }
		};
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

		return StaticMesh{ std::move(t_vertices), std::move(t_faces) };
	}

	StaticModel extractModel(std::pmr::memory_resource* a_resource
		, aiScene const& a_sceneData)
	{
		std::pmr::vector<StaticMesh> t_meshes{
			std::pmr::polymorphic_allocator<StaticMesh>{ a_resource }
		};
		t_meshes.reserve(a_sceneData.mNumMeshes);
		for (auto k = 0u; k < a_sceneData.mNumMeshes; ++k)
		{
			t_meshes.emplace_back(extractMesh(a_resource
				, *a_sceneData.mMeshes[k]));
		}

		return StaticModel{ std::move(t_meshes) };
	}

	AssimpLoader::AssimpLoader(
		Manager<StaticModel>& a_staticModelResourceManager
		, std::pmr::memory_resource* a_memoryResource
	)
		: m_staticModelResourceManager{ a_staticModelResourceManager }
		, m_memoryResource{ a_memoryResource }
	{}

	std::shared_ptr<type::ADynamicType> AssimpLoader::load(std::istream& a_inputStream)
	{
		// Load stream in memory
		auto t_data = data::getData(a_inputStream);
		ignorable_assert(!t_data.empty());

		// Make Assimp load scene
		auto const scene = m_importer.ReadFileFromMemory(t_data.data()
			, t_data.size(), aiProcess_Triangulate | aiProcess_FlipUVs);

		ignorable_assert(scene != nullptr && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE));
		if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			return nullptr;
		}

		return sta::allocate_polymorphic<Handle<StaticModel>>(
			std::pmr::polymorphic_allocator<Handle<StaticModel>>{ m_memoryResource }
			, m_staticModelResourceManager
			, m_memoryResource
			, extractModel(m_memoryResource, *scene)
		);
	}
}
