#include <vob/aoe/rendering/data/model_loader.h>

#include <vob/aoe/data/filesystem_util.h>
#include <vob/aoe/rendering/data/material_data.h>

#include <vob/misc/std/ignorable_assert.h>

#pragma warning( push )
#pragma warning( disable : 26812)
#include <assimp/Importer.hpp>
#include <assimp/pbrmaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#pragma warning( pop )
#include <glm/gtc/type_ptr.hpp>

#include <numeric>
#include <memory>
#include <span>
#include <vector>


namespace
{
	static char const* k_rigRootName = "Root";

	struct temporary_joint
	{
		std::reference_wrapper<aiNode const> m_node;
		std::optional<std::uint8_t> m_parentIndex;
	};

	struct temporary_rig
	{
		std::pmr::vector<temporary_joint> m_joints;

		auto add_joint(aiNode const& a_node, std::optional<std::uint8_t> a_parentIndex)
		{
			m_joints.emplace_back(a_node, a_parentIndex);
			return static_cast<std::int32_t>(m_joints.size() - 1);
		}

		std::optional<std::int8_t> find_joint_index(aiNode const& a_node) const
		{
			auto it = std::find_if(
				m_joints.begin()
				, m_joints.end()
				, [&a_node](auto const& a_temporaryJoint) { return &a_temporaryJoint.m_node.get() == &a_node; });
			if (it == m_joints.end())
			{
				return std::nullopt;
			}

			return static_cast<std::uint8_t>(std::distance(m_joints.begin(), it));
		}
	};

	std::shared_ptr<vob::aoegl::texture_data const> get_texture(
		vob::aoedt::database<vob::aoegl::texture_data> const& a_textureDatabase
		, vob::aoedt::filesystem_indexer const& a_indexer
		, std::filesystem::path const& a_modelPath
		, aiMaterial const& a_materialData
		, aiTextureType a_textureType)
	{
		aiString textureRelativePath;
		if (a_materialData.GetTexture(a_textureType, 0, &textureRelativePath) == aiReturn_FAILURE)
		{
			return nullptr;
		}

		auto const texturePath = vob::aoedt::filesystem_util::normalize(
			textureRelativePath.C_Str(), a_modelPath);

		return a_textureDatabase.find(a_indexer.get_runtime_id(texturePath));
	}

	vob::aoegl::material_data extract_material(
		vob::aoedt::database<vob::aoegl::texture_data> const& a_textureDatabase
		, vob::aoedt::filesystem_indexer const& a_indexer
		, std::filesystem::path const& a_modelPath
		, aiMaterial const& a_materialData)
	{
		vob::aoegl::material_data material;

		material.m_albedo = get_texture(
			a_textureDatabase, a_indexer, a_modelPath, a_materialData, aiTextureType_DIFFUSE);

		material.m_normal = get_texture(
			a_textureDatabase, a_indexer, a_modelPath, a_materialData, aiTextureType_NORMALS);

		material.m_metallicRoughness = get_texture(
			a_textureDatabase, a_indexer, a_modelPath, a_materialData, aiTextureType_UNKNOWN);

		return material;
	}

	std::pmr::vector<vob::aoegl::material_data> extract_materials(
		vob::aoedt::database<vob::aoegl::texture_data> const& a_textureDatabase
		, vob::aoedt::filesystem_indexer const& a_indexer
		, std::filesystem::path const& a_modelPath
		, std::span<aiMaterial const* const> a_materialDataList)
	{
		std::pmr::vector<vob::aoegl::material_data> materials;
		materials.reserve(a_materialDataList.size());

		for (auto const* materialData : a_materialDataList)
		{
			materials.emplace_back(extract_material(
				a_textureDatabase, a_indexer, a_modelPath, *materialData));
		}

		return materials;
	}

	inline glm::vec3 to_glm_vec3(aiVector3D const& a_vector)
	{
		return glm::vec3{ a_vector.x, a_vector.y, a_vector.z };
	}

	inline glm::vec2 to_glm_vec2(aiVector3D const& a_vector)
	{
		return glm::vec2{ a_vector.x, a_vector.y };
	}

	std::pmr::vector<vob::aoegl::triangle> extract_triangles(std::span<aiFace> a_faces)
	{
		std::pmr::vector<vob::aoegl::triangle> triangles;
		if (a_faces.size() == 0 || a_faces[0].mNumIndices != 3)
		{
			ignorable_assert(false && "Models with non-triangular faces not supported.");
			return triangles;
		}
		triangles.reserve(a_faces.size());

		for (auto const& face : a_faces)
		{
			triangles.emplace_back(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
		}
		return triangles;
	}

	inline glm::mat4 to_glm_mat4(aiMatrix4x4 const& a_matrix)
	{
		return glm::transpose(glm::make_mat4(std::addressof(a_matrix.a1)));
	}

	std::pmr::vector<glm::vec3> extract_vectors(std::span<aiVector3D> a_aiVector3DList)
	{
		static_assert(sizeof(aiVector3D) == sizeof(glm::vec3));
		static_assert(alignof(aiVector3D) == alignof(glm::vec3));
		static_assert(offsetof(aiVector3D, x) == offsetof(glm::vec3, x));
		static_assert(offsetof(aiVector3D, y) == offsetof(glm::vec3, y));
		static_assert(offsetof(aiVector3D, z) == offsetof(glm::vec3, z));

		std::pmr::vector<glm::vec3> vec3List{ a_aiVector3DList.size() };
		std::memcpy(
			vec3List.data(), a_aiVector3DList.data(), a_aiVector3DList.size() * sizeof(glm::vec3));
		return vec3List;
	}

	std::pmr::vector<glm::vec2> extract_vectors_2d(std::span<aiVector3D> a_aiVector3DList)
	{
		std::pmr::vector<glm::vec2> vec2List{ a_aiVector3DList.size() };
		for (auto i = 0u; i < a_aiVector3DList.size(); ++i)
		{
			vec2List[i].x = a_aiVector3DList[i].x;
			vec2List[i].y = a_aiVector3DList[i].y;
		}
		return vec2List;
	}

#ifdef _TODO_ANIMATION_
	std::pmr::vector<vob::aoegl::vertex_weight> extract_vertex_weights(
		std::span<aiVertexWeight> a_vertexWeights)
	{
		std::pmr::vector<vob::aoegl::vertex_weight> vertexWeights;
		vertexWeights.reserve(a_vertexWeights.size());
		for (auto const& vertexWeight : a_vertexWeights)
		{
			vertexWeights.emplace_back(vertexWeight.mVertexId, vertexWeight.mWeight);
		}
		return vertexWeights;
	}

	std::pmr::vector<vob::aoegl::mesh_joint> extract_mesh_joints(std::span<aiBone*> a_joints)
	{
		std::pmr::vector<vob::aoegl::mesh_joint> meshJoints;
		meshJoints.reserve(a_joints.size());
		for (auto const* joint : a_joints)
		{
			meshJoints.emplace_back(
				joint->mName.C_Str(),
				to_glm_mat4(joint->mOffsetMatrix),
				extract_vertex_weights(std::span{ joint->mWeights, joint->mNumWeights }));
		}
		return meshJoints;
	}
#endif

	vob::aoegl::mesh_data extract_mesh(aiMesh const& a_meshData)
	{
		vob::aoegl::mesh_data mesh;

		mesh.m_positions = extract_vectors(
			std::span{ a_meshData.mVertices, a_meshData.mNumVertices });
		mesh.m_textureCoords = extract_vectors_2d(
			std::span{ a_meshData.mTextureCoords[0], a_meshData.mNumVertices });
		mesh.m_normals = extract_vectors(
			std::span{ a_meshData.mNormals, a_meshData.mNumVertices });
		mesh.m_tangents = extract_vectors(
			std::span{ a_meshData.mTangents, a_meshData.mNumVertices });

#ifdef _TODO_ANIMATION_
		mesh.m_joints = extract_mesh_joints(std::span{ a_meshData.mBones, a_meshData.mNumBones });
#endif

		mesh.m_triangles = extract_triangles(std::span{ a_meshData.mFaces, a_meshData.mNumFaces });

		return mesh;
	}

	vob::aoegl::textured_mesh_data extract_textured_mesh(
		std::pmr::vector<vob::aoegl::material_data> const& a_materials, aiMesh const& a_meshData)
	{
		vob::aoegl::textured_mesh_data texturedMesh;
		
		texturedMesh.m_mesh = extract_mesh(a_meshData);

		texturedMesh.m_material = a_materials[a_meshData.mMaterialIndex];

		return texturedMesh;
	}

	std::pmr::vector<vob::aoegl::textured_mesh_data> extract_textured_meshes(
		std::pmr::vector<vob::aoegl::material_data> const& a_materials,
		std::span<aiMesh const* const> a_meshDataList)
	{
		std::pmr::vector<vob::aoegl::textured_mesh_data> texturedMeshes;
		texturedMeshes.reserve(a_meshDataList.size());
		for (auto const* meshData : a_meshDataList)
		{
			texturedMeshes.emplace_back(extract_textured_mesh(
				a_materials, *meshData));
		}
		return texturedMeshes;
	}

	vob::aoegl::model_data extract_model(
		vob::aoedt::database<vob::aoegl::texture_data> const& a_textureDatabase,
		vob::aoedt::filesystem_indexer const& a_indexer,
		std::filesystem::path const& a_modelPath,
		aiScene const& a_sceneData)
	{
		std::pmr::vector<vob::aoegl::material_data> materials = extract_materials(
			a_textureDatabase
			, a_indexer
			, a_modelPath
			, std::span{ a_sceneData.mMaterials, a_sceneData.mNumMaterials });

		std::pmr::vector<vob::aoegl::textured_mesh_data> texturedMeshes = extract_textured_meshes(
				materials, std::span{ a_sceneData.mMeshes, a_sceneData.mNumMeshes });

		return { texturedMeshes };
	}
}

namespace vob::aoegl
{
	model_data model_loader::load(std::filesystem::path const& a_path) const
	{
		Assimp::Importer importer;
		auto const scene = importer.ReadFile(
			a_path.generic_string()
			, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
				| aiProcess_LimitBoneWeights | aiProcess_PopulateArmatureData);
		if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			return {};
		}


		auto id = m_indexer.get_runtime_id(a_path);

		return extract_model(m_textureDatabase, m_indexer, a_path, *scene);
	}
}