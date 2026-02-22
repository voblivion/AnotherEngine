#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <vector>


namespace vob::aoegl
{
	struct StaticVertexData
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec3 tangent;
	};

	struct StaticMeshData
	{
		std::pmr::vector<StaticVertexData> vertices;
		std::pmr::vector<uint32_t> indices;
	};

	struct StaticModelData
	{
		std::pmr::vector<StaticMeshData> meshes;
	};

	struct RiggedVertexData
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec3 tangent;
		glm::ivec4 boneIndices;
		glm::vec4 boneWeights;
	};

	struct RiggedMeshData
	{
		std::pmr::vector<RiggedVertexData> vertices;
		std::pmr::vector<uint32_t> indices;
	};

	struct BoneData
	{
		std::pmr::string name;
		int32_t parentBoneIndex = 0;
		glm::mat4 basePose = glm::mat4{ 1.0f };
	};

	struct RiggedModelData
	{
		std::pmr::vector<RiggedMeshData> meshes;
		std::pmr::vector<BoneData> bones;
	};
}
