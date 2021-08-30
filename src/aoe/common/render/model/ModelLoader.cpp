#include <vob/aoe/common/render/model/ModelLoader.h>

#include <vob/sta/ignorable_assert.h>
#include <assimp/postprocess.h>
#include <vob/aoe/core/data/LoaderUtils.h>
#include <assimp/scene.h>
#include <vob/aoe/common/render/GraphicResourceHandle.h>
#include <iostream>
#include <string_view>
#include <vob/aoe/common/data/filesystem/FileSystemIndexer.h>

namespace vob::aoe::common
{
	inline glm::vec3 toGlmVec3(aiVector3D const& a_vector)
	{
		return glm::vec3{ a_vector.x, a_vector.y, a_vector.z };
	}

	inline glm::vec2 toGlmVec2(aiVector2D const& a_vector)
	{
		return glm::vec2{ a_vector.x, a_vector.y };
	}

	inline glm::vec2 toGlmVec2(aiVector3D const& a_vector)
	{
		return glm::vec2{ a_vector.x, a_vector.y };
	}

	std::vector<Vertex> extractVertices(aiMesh const& a_meshData)
	{
		std::vector<Vertex> t_vertices{};

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

	StaticMesh extractMesh(aiMesh const& a_meshData)
	{
		auto t_vertices = extractVertices(a_meshData);

		// Load faces, only triangles supported
		std::vector<Triangle> t_faces{};
		t_faces.reserve(a_meshData.mNumFaces);
		if (a_meshData.mFaces->mNumIndices == 3)
		{
			for (auto k = 0u; k < a_meshData.mNumFaces; ++k)
			{
				t_faces.emplace_back(
					Triangle{ a_meshData.mFaces[k].mIndices[0]
					, a_meshData.mFaces[k].mIndices[1]
					, a_meshData.mFaces[k].mIndices[2] }
				);
			}
		}

		return StaticMesh{ std::move(t_vertices), std::move(t_faces), a_meshData.mMaterialIndex };
	}

	Material extractMaterial(
		data::ADatabase& a_database
		, FileSystemIndexer& a_fileSystemIndexer
		, aiMaterial const& a_materialData
		, std::filesystem::path const& a_loadingDataPath
	)
	{
		/*std::cout << "================================" << std::endl;
		std::cout << "Diffuse count: " << a_materialData.GetTextureCount(aiTextureType_DIFFUSE) << std::endl;
		std::cout << "Specular count: " << a_materialData.GetTextureCount(aiTextureType_SPECULAR) << std::endl;
		std::cout << "Normals count: " << a_materialData.GetTextureCount(aiTextureType_NORMALS) << std::endl;
		std::cout << "Properties:" << std::endl;
		for (auto j = 0u; j < a_materialData.mNumProperties; ++j)
		{
			std::cout << "--------------------------------" << std::endl;
			auto const materialProperty = a_materialData.mProperties[j];
			std::cout << "Type: " << materialProperty->mType << std::endl;
			std::cout << "Name: " << materialProperty->mKey.C_Str() << std::endl;
			std::cout << "Raw: ";
			for (auto k = 0u; k < materialProperty->mDataLength; ++k)
			{
				unsigned char const c = materialProperty->mData[k];
				std::cout << std::hex << static_cast<unsigned>(c) << " ";
			}
			std::cout << std::endl;
			std::cout << "Str: " << std::string_view{ materialProperty->mData, materialProperty->mDataLength } << std::endl;
		}*/

		Material material;

		aiString diffusePathStr;
		if (a_materialData.GetTexture(aiTextureType_DIFFUSE, 0, &diffusePathStr) == aiReturn_SUCCESS)
		{
			auto const diffusePath = common::pathFromFilePath(diffusePathStr.C_Str(), a_loadingDataPath);
			a_database.find(a_fileSystemIndexer.getId(diffusePath), material.m_diffuse);
		}

		aiString specularPathStr;
		if (a_materialData.GetTexture(aiTextureType_SPECULAR, 0, &specularPathStr) == aiReturn_SUCCESS)
		{
			auto const specularPath = common::pathFromFilePath(specularPathStr.C_Str(), a_loadingDataPath);
			 a_database.find(a_fileSystemIndexer.getId(specularPath), material.m_specular);
		}

		return material;
	}

	StaticModel extractModel(
		data::ADatabase& a_database
		, FileSystemIndexer& a_fileSystemIndexer
		, aiScene const& a_sceneData
		, std::filesystem::path const& a_loadingDataPath
	)
	{
		std::vector<StaticMesh> t_meshes{};
		t_meshes.reserve(a_sceneData.mNumMeshes);
		for (auto k = 0u; k < a_sceneData.mNumMeshes; ++k)
		{
			t_meshes.emplace_back(
				extractMesh(*a_sceneData.mMeshes[k])
			);
		}

		std::vector<Material> t_materials{};
		t_materials.reserve(a_sceneData.mNumMaterials);
		for (auto k = 0u; k < a_sceneData.mNumMaterials; ++k)
		{
			t_materials.emplace_back(
				extractMaterial(
					a_database
					, a_fileSystemIndexer
					, *a_sceneData.mMaterials[k]
					, a_loadingDataPath
				)
			);
		}

		return StaticModel{ std::move(t_meshes), t_materials };
	}

	ModelLoader::ModelLoader(
		data::ADatabase& a_database
		, FileSystemIndexer& a_fileSystemIndexer
		, IGraphicResourceManager<StaticModel>& a_staticModelResourceManager
	)
		: m_database{ a_database }
		, m_fileSystemIndexer{ a_fileSystemIndexer }
		, m_staticModelResourceManager{ a_staticModelResourceManager }
	{}

	aiScene const* rawLoad(Assimp::Importer& a_importer, std::filesystem::path const& a_path)
	{
		auto const scene = a_importer.ReadFile(
			a_path.generic_string()
			, aiProcess_Triangulate | aiProcess_FlipUVs
		);

		return (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) ? nullptr : scene;
	}

	bool ModelLoader::canLoad(std::filesystem::path const& a_path) const
	{
		Assimp::Importer importer;
		return importer.IsExtensionSupported(a_path.extension().generic_string().c_str());
	}


	std::shared_ptr<type::ADynamicType> ModelLoader::load(std::filesystem::path const& a_path) const
	{
		Assimp::Importer t_importer;
		auto const scene = rawLoad(t_importer, a_path);

		ignorable_assert(scene != nullptr);
		if (scene == nullptr)
		{
			return nullptr;
		}

		return std::make_shared<GraphicResourceHandle<StaticModel>>(
			m_staticModelResourceManager
			, extractModel(m_database, m_fileSystemIndexer, *scene, a_path)
		);
	}
}
