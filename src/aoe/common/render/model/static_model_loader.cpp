#include <vob/aoe/common/_render/model/static_model_loader.h>

#include <vob/misc/std/ignorable_assert.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>
#include <vob/aoe/core/data/LoaderUtils.h>
#include <assimp/scene.h>
#include <vob/aoe/common/_render/GraphicResourceHandle.h>
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

	std::vector<static_vertex> extractVertices(aiMesh const& a_meshData)
	{
		std::vector<static_vertex> t_vertices{};

		t_vertices.reserve(a_meshData.mNumVertices);
		if (a_meshData.mTextureCoords[0] != nullptr)
		{
			for (auto k = 0u; k < a_meshData.mNumVertices; ++k)
			{
				static_vertex t{
					toGlmVec3(a_meshData.mVertices[k])
					, toGlmVec3(a_meshData.mNormals[k])
					, toGlmVec2(a_meshData.mTextureCoords[0][k])
					, toGlmVec3(a_meshData.mTangents[k])
				};
				t_vertices.emplace_back(t);
			}
		}
		else
		{
			for (auto k = 0u; k < a_meshData.mNumVertices; ++k)
			{
				static_vertex t{ toGlmVec3(a_meshData.mVertices[k])
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
		std::vector<triangle> t_faces{};
		t_faces.reserve(a_meshData.mNumFaces);
		if (a_meshData.mFaces->mNumIndices == 3)
		{
			for (auto k = 0u; k < a_meshData.mNumFaces; ++k)
			{
				t_faces.emplace_back(
					triangle{ a_meshData.mFaces[k].mIndices[0]
					, a_meshData.mFaces[k].mIndices[1]
					, a_meshData.mFaces[k].mIndices[2] }
				);
			}
		}

		for (auto b = 0u; b < a_meshData.mNumBones; ++b)
		{
			auto& bone = *a_meshData.mBones[b];
			mishs::string_id id{ bone.mName.C_Str() };
			for (auto w = 0u; w < bone.mNumWeights; ++w)
			{
				auto& weight = bone.mWeights[w];
				if (true)
				{

				}
			}
		}

		return StaticMesh{ std::move(t_vertices), std::move(t_faces), a_meshData.mMaterialIndex };
	}

	old_material extractMaterial(
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

		old_material material;

		std::cout << a_loadingDataPath << std::endl;

		aiString name = a_materialData.GetName();

		aiString albedoRelativePathStr;
		if (a_materialData.GetTexture(aiTextureType_DIFFUSE, 0, &albedoRelativePathStr) == aiReturn_SUCCESS)
		{
			auto const albedoRelativePath = common::pathFromFilePath(albedoRelativePathStr.C_Str(), a_loadingDataPath);
			a_database.find(a_fileSystemIndexer.get_id(albedoRelativePath), material.m_albedo);
		}

		aiString normalRelativePathStr;
		if (a_materialData.GetTexture(aiTextureType_NORMALS, 0, &normalRelativePathStr) == aiReturn_SUCCESS)
		{
			auto const normalRelativePath = common::pathFromFilePath(normalRelativePathStr.C_Str(), a_loadingDataPath);
			a_database.find(a_fileSystemIndexer.get_id(normalRelativePath), material.m_normal);
		}

		aiString metallicRoughnessRelativePathStr;
		if (a_materialData.GetTexture(aiTextureType_UNKNOWN, 0, &metallicRoughnessRelativePathStr) == aiReturn_SUCCESS)
		{
			auto const metallicRoughnessRelativePath = common::pathFromFilePath(
				metallicRoughnessRelativePathStr.C_Str(), a_loadingDataPath);
			a_database.find(a_fileSystemIndexer.get_id(metallicRoughnessRelativePath), material.m_metallicRoughness);
		}

		return material;
	}

	static_model extractModel(
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

		for (auto k = 0u; k < a_sceneData.mNumAnimations; ++k)
		{
			auto& animation = *a_sceneData.mAnimations[k];
			if (true)
			{

			}
		}

		std::vector<old_material> t_materials{};
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

		return static_model{ std::move(t_meshes), t_materials };
	}

	static_model_loader::static_model_loader(
		data::ADatabase& a_database
		, FileSystemIndexer& a_fileSystemIndexer
		, IGraphicResourceManager<static_model>& a_staticModelResourceManager
	)
		: m_database{ a_database }
		, m_fileSystemIndexer{ a_fileSystemIndexer }
		, m_staticModelResourceManager{ a_staticModelResourceManager }
	{}

	aiScene const* rawLoad(Assimp::Importer& a_importer, std::filesystem::path const& a_path)
	{
		auto const scene = a_importer.ReadFile(
			a_path.generic_string()
			, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
		);

		return (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) ? nullptr : scene;
	}

	bool static_model_loader::canLoad(std::filesystem::path const& a_path) const
	{
		Assimp::Importer importer;
		return importer.IsExtensionSupported(a_path.extension().generic_string().c_str());
	}


	std::shared_ptr<type::ADynamicType> static_model_loader::load(std::filesystem::path const& a_path) const
	{
		Assimp::Importer t_importer;
		auto const scene = rawLoad(t_importer, a_path);

		ignorable_assert(scene != nullptr);
		if (scene == nullptr)
		{
			return nullptr;
		}

		return std::make_shared<GraphicResourceHandle<static_model>>(
			m_staticModelResourceManager
			, extractModel(m_database, m_fileSystemIndexer, *scene, a_path)
		);
	}
}
