#include <vob/aoe/animation/rig_loader.h>

#pragma warning( push )
#pragma warning( disable : 26812)
#include <assimp/Importer.hpp>
#include <assimp/pbrmaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#pragma warning( pop )
#include <glm/gtc/type_ptr.hpp>

#include <span>


namespace
{
	inline glm::vec3 to_glm_vec3(aiVector3D const& a_vector)
	{
		return glm::vec3{ a_vector.x, a_vector.y, a_vector.z };
	}

	inline glm::mat4 to_glm_mat4(aiMatrix4x4 const& a_matrix)
	{
		return glm::transpose(glm::make_mat4(std::addressof(a_matrix.a1)));
	}

	inline glm::quat to_glm_quat(aiQuaternion const& a_quaternion)
	{
		return glm::quat{ a_quaternion.x, a_quaternion.y, a_quaternion.z, a_quaternion.w };
	}

	std::size_t calculate_hierarchy_size(aiNode const& a_node)
	{
		auto hierarchySize = std::size_t{ 1 };
		for (auto childNode : std::span{ a_node.mChildren, a_node.mNumChildren })
		{
			hierarchySize += calculate_hierarchy_size(*childNode);
		}
		return hierarchySize;
	}

	void load_joints_rec(
		aiNode const& a_node
		, std::optional<std::size_t> a_parentIndex
		, std::pmr::vector<vob::aoegl::joint_data>& a_joints)
	{
		a_joints.emplace_back(a_parentIndex, a_node.mName.C_Str(), to_glm_mat4(a_node.mTransformation));
		a_parentIndex = a_joints.size() - 1;
		for (auto childNode : std::span{ a_node.mChildren, a_node.mNumChildren })
		{
			load_joints_rec(*childNode, a_parentIndex, a_joints);
		}
	}

	struct quat_key
	{
		float m_time;
		glm::quat m_value;
	};

	struct vector_key
	{
		float m_time;
		glm::vec3 m_value;
	};

	quat_key extract_quat_key(aiQuatKey const& a_quatKeyData)
	{
		return { static_cast<float>(a_quatKeyData.mTime), to_glm_quat(a_quatKeyData.mValue) };
	}

	std::pmr::vector<quat_key> extract_quat_keys(std::span<aiQuatKey> a_quatKeysData)
	{
		std::pmr::vector<quat_key> quatKeys;
		quatKeys.reserve(a_quatKeysData.size());
		for (auto const& quatKeyData : a_quatKeysData)
		{
			quatKeys.push_back(extract_quat_key(quatKeyData));
		}
		return quatKeys;
	}

	vector_key extract_vector_key(aiVectorKey const& a_vectorKeyData)
	{
		return { static_cast<float>(a_vectorKeyData.mTime), to_glm_vec3(a_vectorKeyData.mValue) };
	}

	std::pmr::vector<vector_key> extract_vector_keys(std::span<aiVectorKey> a_vectorKeysData)
	{
		std::pmr::vector<vector_key> vectorKeys;
		vectorKeys.reserve(a_vectorKeysData.size());
		for (auto const& vectorKeyData : a_vectorKeysData)
		{
			vectorKeys.push_back(extract_vector_key(vectorKeyData));
		}
		return vectorKeys;
	}

	void extract_animation_node(aiNodeAnim const& a_animationNodeData)
	{
		const std::pmr::string name{ a_animationNodeData.mNodeName.C_Str() };

		auto const positionKeys = extract_vector_keys(
			std::span{ a_animationNodeData.mPositionKeys, a_animationNodeData.mNumPositionKeys });

		auto const rotationKeys = extract_quat_keys(
			std::span{ a_animationNodeData.mRotationKeys, a_animationNodeData.mNumRotationKeys });

		auto const scalingKeys = extract_vector_keys(
			std::span{ a_animationNodeData.mScalingKeys, a_animationNodeData.mNumScalingKeys });
	}

	void extract_animation_nodes(std::span<aiNodeAnim*> a_animationNodesData)
	{
		for (auto const* animationNodeData : a_animationNodesData)
		{
			extract_animation_node(*animationNodeData);
		}
	}

	void extract_animation(aiAnimation const& a_animationData)
	{
		const std::pmr::string name{ a_animationData.mName.C_Str() };
		const float duration = static_cast<float>(a_animationData.mDuration);
		const float ticksPerSecond = static_cast<float>(a_animationData.mTicksPerSecond);
		extract_animation_nodes(std::span{ a_animationData.mChannels, a_animationData.mNumChannels });
	}

	void extract_animations(std::span<aiAnimation*> a_animationsData)
	{
		for (auto const* animationData : a_animationsData)
		{
			extract_animation(*animationData);
		}
	}
}

namespace vob::aoegl
{
	static constexpr char const* k_rootJointName = "Root";

	rig_data rig_loader::load(std::filesystem::path const& a_path) const
	{
		Assimp::Importer importer;
		auto const scene = importer.ReadFile(
			a_path.generic_string()
			, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_LimitBoneWeights);

		if (scene == nullptr)
		{
			return {};
		}

		aiNode const* rigRoot = scene->mRootNode->FindNode(k_rootJointName);
		if (rigRoot == nullptr)
		{
			return {};
		}

		extract_animations(std::span{ scene->mAnimations, scene->mNumAnimations });

		auto rigData = rig_data{};
		rigData.m_joints.reserve(calculate_hierarchy_size(*rigRoot));
		load_joints_rec(*rigRoot, std::nullopt, rigData.m_joints);
		return rigData;
	}
}
