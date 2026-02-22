#include <vob/aoe/rendering/ModelLoader.h>

#include <vob/misc/std/ignorable_assert.h>

#pragma warning( push )
#pragma warning( disable : 26812)
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#pragma warning( pop )

#include <cstdint>
#include <filesystem>
#include <span>
#include <unordered_map>
#include <vector>


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

		StaticMeshData extractStaticMesh(aiMesh const& a_meshData)
		{
			StaticMeshData staticMesh;
			staticMesh.vertices.reserve(a_meshData.mNumVertices);
			for (int32_t i = 0; i < static_cast<int32_t>(a_meshData.mNumVertices); ++i)
			{
				staticMesh.vertices.emplace_back(
					toVec3(*(a_meshData.mVertices + i)),
					toVec3(*(a_meshData.mNormals + i)),
					toVec2(*(a_meshData.mTextureCoords[0] + i)),
					toVec3(*(a_meshData.mTangents + i)));
			}
			for (int32_t i = 0; i < a_meshData.mNumFaces; ++i)
			{
				staticMesh.indices.emplace_back(a_meshData.mFaces[i].mIndices[0]);
				staticMesh.indices.emplace_back(a_meshData.mFaces[i].mIndices[1]);
				staticMesh.indices.emplace_back(a_meshData.mFaces[i].mIndices[2]);
			}

			return staticMesh;
		}

		void extractStaticModelRec(
			aiScene const& a_sceneData,
			aiNode const& a_nodeData,
			glm::mat4 const& a_parentTransform,
			StaticModelData& o_staticModel)
		{
			glm::mat4 transform = a_parentTransform * toMat4(a_nodeData.mTransformation);
			for (auto const meshIndex : std::span<uint32_t>{ a_nodeData.mMeshes, a_nodeData.mNumMeshes })
			{
				auto const& meshData = *a_sceneData.mMeshes[meshIndex];
				auto part = extractStaticMesh(meshData);
				for (auto& vertex : part.vertices)
				{
					vertex.position = glm::vec3{ transform * glm::vec4{ vertex.position, 1.0f } };
					vertex.normal = glm::vec3{ transform * glm::vec4{vertex.normal, 0.0f} };
				}

				o_staticModel.meshes.emplace_back(std::move(part));
			}

			for (auto const childNodeData : std::span<aiNode*>{ a_nodeData.mChildren, a_nodeData.mNumChildren })
			{
				extractStaticModelRec(a_sceneData, *childNodeData, transform, o_staticModel);
			}
		}

		StaticModelData extractStaticModel(aiScene const& a_sceneData)
		{
			StaticModelData staticModel;
			extractStaticModelRec(a_sceneData, *a_sceneData.mRootNode, glm::mat4(1.0f), staticModel);
			return staticModel;
		}

		RiggedMeshData extractRiggedMesh(aiMesh const& a_meshData, std::vector<aiNode*>& o_boneNodes)
		{
			RiggedMeshData riggedMesh;
			riggedMesh.vertices.reserve(a_meshData.mNumVertices);
			for (auto i = 0; i < static_cast<int32_t>(a_meshData.mNumVertices); ++i)
			{
				glm::ivec4 boneIndices{ 0 };
				glm::vec4 boneWeights{ 0.0f };
				riggedMesh.vertices.emplace_back(
					toVec3(a_meshData.mVertices[i]),
					toVec3(a_meshData.mNormals[i]),
					toVec2(a_meshData.mTextureCoords[0][i]),
					toVec3(a_meshData.mTangents[i]),
					boneIndices,
					boneWeights);
			}

			for (auto b = 0; b < static_cast<int32_t>(a_meshData.mNumBones); ++b)
			{
				auto const bone = a_meshData.mBones[b];

				for (auto const& vertexWeight : std::span{ bone->mWeights, bone->mNumWeights })
				{
					if (vertexWeight.mWeight != 0.0f)
					{
						continue;
					}
				}

				auto const boneNodeIt = std::find_if(
					o_boneNodes.begin(), o_boneNodes.end(), [bone](const aiNode* boneNode) { return boneNode == bone->mNode; });
				auto const boneIndex = std::distance(o_boneNodes.begin(), boneNodeIt);
				if (boneNodeIt == o_boneNodes.end())
				{
					if (bone->mNode->mNumChildren == 0)
					{
						bool hasWeight = false;
						for (auto const& vertexWeight : std::span{ bone->mWeights, bone->mNumWeights })
						{
							if (vertexWeight.mWeight != 0.0f)
							{
								hasWeight = true;
								break;
							}
						}

						if (!hasWeight)
						{
							continue;
						}
					}

					o_boneNodes.emplace_back(bone->mNode);
				}
				
				for (auto const& vertexWeight : std::span{ bone->mWeights, bone->mNumWeights })
				{
					if (vertexWeight.mWeight == 0.0f)
					{
						continue;
					}

					for (auto w = 0; w < 4; ++w)
					{
						if (riggedMesh.vertices[vertexWeight.mVertexId].boneWeights[w] == 0.0f)
						{
							riggedMesh.vertices[vertexWeight.mVertexId].boneIndices[w] = boneIndex;
							riggedMesh.vertices[vertexWeight.mVertexId].boneWeights[w] = vertexWeight.mWeight;
							break;
						}
					}
				}
			}

			for (auto i = 0; i < a_meshData.mNumFaces; ++i)
			{
				riggedMesh.indices.emplace_back(a_meshData.mFaces[i].mIndices[0]);
				riggedMesh.indices.emplace_back(a_meshData.mFaces[i].mIndices[1]);
				riggedMesh.indices.emplace_back(a_meshData.mFaces[i].mIndices[2]);
			}

			return riggedMesh;
		}

		void extractRiggedModelRec(
			aiScene const& a_sceneData,
			aiNode const& a_nodeData,
			glm::mat4 const& a_parentTransform,
			RiggedModelData& o_riggedModel,
			std::vector<aiNode*>& o_boneNodes)
		{
			glm::mat4 transform = a_parentTransform * toMat4(a_nodeData.mTransformation);
			for (auto const meshIndex : std::span<uint32_t>{ a_nodeData.mMeshes, a_nodeData.mNumMeshes })
			{
				auto const& meshData = *a_sceneData.mMeshes[meshIndex];
				auto part = extractRiggedMesh(meshData, o_boneNodes);
				for (auto& vertex : part.vertices)
				{
					vertex.position = glm::vec3{ transform * glm::vec4{ vertex.position, 1.0f } };
					vertex.normal = glm::vec3{ transform * glm::vec4{vertex.normal, 0.0f} };
				}

				o_riggedModel.meshes.emplace_back(std::move(part));
			}

			for (auto const childNodeData : std::span<aiNode*>{ a_nodeData.mChildren, a_nodeData.mNumChildren })
			{
				extractRiggedModelRec(a_sceneData, *childNodeData, transform, o_riggedModel, o_boneNodes);
			}
		}

		void extractRigBasePoseRec(
			aiNode const& a_nodeData,
			glm::mat4 const& a_parentTransform,
			int32_t a_parentBoneIndex,
			RiggedModelData& o_riggedModel,
			std::vector<aiNode*>& o_boneNodes)
		{
			glm::mat4 transform = a_parentTransform * toMat4(a_nodeData.mTransformation);
			auto boneIndex = a_parentBoneIndex;
			for (auto i = 0; i < o_boneNodes.size(); ++i)
			{
				if (&a_nodeData == o_boneNodes[i])
				{
					auto& bone = o_riggedModel.bones[i];
					bone.name = std::pmr::string{ std::string_view{ a_nodeData.mName.data, a_nodeData.mName.length } };
					bone.basePose = transform;
					bone.parentBoneIndex = a_parentBoneIndex;
					boneIndex = i;
					break;
				}
			}

			for (auto const childNodeData : std::span<aiNode*>{ a_nodeData.mChildren, a_nodeData.mNumChildren })
			{
				extractRigBasePoseRec(
					*childNodeData, transform, boneIndex, o_riggedModel, o_boneNodes);
			}
		}

		RiggedModelData extractRiggedModel(aiScene const& a_sceneData)
		{
			RiggedModelData riggedModel;
			std::vector<aiNode*> boneNodes;
			extractRiggedModelRec(a_sceneData, *a_sceneData.mRootNode, glm::mat4(1.0f), riggedModel, boneNodes);
			
			riggedModel.bones.resize(boneNodes.size());
			extractRigBasePoseRec(*a_sceneData.mRootNode, glm::mat4(1.0f), 0, riggedModel, boneNodes);

			return riggedModel;
		}
	}

	StaticModelData StaticModelLoader::load(std::filesystem::path const& a_path) const
	{
		Assimp::Importer importer;
		auto const* scene = importer.ReadFile(
			a_path.generic_string(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			ignorable_assert(false && "Invalid model asset");
			return {};
		}

		return extractStaticModel(*scene);
	}

	RiggedModelData RiggedModelLoader::load(std::filesystem::path const& a_path) const
	{
		Assimp::Importer importer;
		auto const* scene = importer.ReadFile(
			a_path.generic_string(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_PopulateArmatureData);
		if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			ignorable_assert(false && "Invalid model asset");
			return {};
		}

		return extractRiggedModel(*scene);
	}
}
