#include <vob/aoe/rendering/StaticMeshLoader.h>

#include <vob/misc/std/ignorable_assert.h>

#pragma warning( push )
#pragma warning( disable : 26812)
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#pragma warning( pop )

#include <span>


namespace vob::aoegl
{
	namespace
	{
		static_assert(sizeof(aiVector3D) == sizeof(glm::vec3));
		static_assert(alignof(aiVector3D) == alignof(glm::vec3));
		static_assert(offsetof(aiVector3D, x) == offsetof(glm::vec3, x));
		static_assert(offsetof(aiVector3D, y) == offsetof(glm::vec3, y));
		static_assert(offsetof(aiVector3D, z) == offsetof(glm::vec3, z));

		glm::vec3 toVec3(aiVector3D const& a_vector)
		{
			return glm::vec3{ a_vector.x, a_vector.y, a_vector.z };
		}

		glm::vec2 toVec2(aiVector3D const& a_vector)
		{
			return glm::vec2{ a_vector.x, a_vector.y };
		}

		glm::mat4 toMat4(aiMatrix4x4 const& a_matrix)
		{
			return glm::mat4{
				a_matrix.a1, a_matrix.b1, a_matrix.c1, a_matrix.d1,
				a_matrix.a2, a_matrix.b2, a_matrix.c2, a_matrix.d2,
				a_matrix.a3, a_matrix.b3, a_matrix.c3, a_matrix.d3,
				a_matrix.a4, a_matrix.b4, a_matrix.c4, a_matrix.d4
			};
		}

		StaticMesh::Part extractMeshPart(aiMesh const& a_meshData)
		{
			StaticMesh::Part staticMeshPart;
			staticMeshPart.vertices.reserve(a_meshData.mNumVertices);
			for (int32_t i = 0; i < static_cast<int32_t>(a_meshData.mNumVertices); ++i)
			{
				staticMeshPart.vertices.emplace_back(
					toVec3(*(a_meshData.mVertices + i)),
					toVec3(*(a_meshData.mNormals + i)),
					toVec2(*(a_meshData.mTextureCoords[0] + i)),
					toVec3(*(a_meshData.mTangents + i)));
			}
			for (int32_t i = 0; i < static_cast<int32_t>(a_meshData.mNumFaces); ++i)
			{
				staticMeshPart.indices.emplace_back(a_meshData.mFaces[i].mIndices[0]);
				staticMeshPart.indices.emplace_back(a_meshData.mFaces[i].mIndices[1]);
				staticMeshPart.indices.emplace_back(a_meshData.mFaces[i].mIndices[2]);
			}

			return staticMeshPart;
		}

		void extractNodeRec(aiScene const& a_sceneData, aiNode const& a_nodeData, glm::mat4 const& a_parentTransform, StaticMesh& o_staticMesh)
		{
			glm::mat4 transform = a_parentTransform * toMat4(a_nodeData.mTransformation);
			for (auto const meshIndex : std::span<uint32_t>{ a_nodeData.mMeshes, a_nodeData.mNumMeshes })
			{
				auto const& meshData = *a_sceneData.mMeshes[meshIndex];
				auto part = extractMeshPart(meshData);
				for (auto& vertex : part.vertices)
				{
					vertex.position = glm::vec3{ transform * glm::vec4{ vertex.position, 1.0f } };
					vertex.normal = glm::vec3{ transform * glm::vec4{vertex.normal, 0.0f} };
				}

				o_staticMesh.parts.emplace_back(std::move(part));
			}

			for (auto const childNodeData : std::span<aiNode*>{ a_nodeData.mChildren, a_nodeData.mNumChildren })
			{
				extractNodeRec(a_sceneData, *childNodeData, transform, o_staticMesh);
			}
		}

		StaticMesh extractMesh(aiScene const& a_sceneData)
		{
			StaticMesh staticMesh;
			extractNodeRec(a_sceneData, *a_sceneData.mRootNode, glm::mat4(1.0f), staticMesh);
			return staticMesh;
		}
	}

	StaticMesh StaticMeshLoader::load(std::filesystem::path const& a_path) const
	{
		Assimp::Importer importer;
		auto const* scene = importer.ReadFile(
			a_path.generic_string(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			ignorable_assert(false && "Invalid mesh asset");
			return {};
		}

		return extractMesh(*scene);
	}
}
