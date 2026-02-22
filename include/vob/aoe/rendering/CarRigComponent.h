#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <vector>


namespace vob::aoegl
{
	struct CarRigComponent
	{
		struct SuspensionWheel
		{
			int32_t wheelIndex = 0;
			int32_t boneIndex = 0;
			int32_t parentBoneIndex = 0;
			glm::mat4 invBasePose = glm::mat4{ 1.0f };
			glm::mat4 baseTransformRelativeParent = glm::mat4{ 1.0f };
		};

		struct SteeringWheel
		{
			int32_t wheelIndex = 0;
			int32_t boneIndex = 0;
			int32_t parentBoneIndex = 0;
			glm::mat4 invBasePose = glm::mat4{ 1.0f };
			glm::mat4 baseTransformRelativeParent = glm::mat4{ 1.0f };
		};

		struct SpinningWheel
		{
			float wheelRadius = 0.0f;
			int32_t boneIndex = 0;
			int32_t parentBoneIndex = 0;
			glm::mat4 invBasePose = glm::mat4{ 1.0f };
			glm::mat4 baseTransformRelativeParent = glm::mat4{ 1.0f };
			float distance = 0.0f;
		};

		std::vector<glm::mat4> bones;
		std::vector<glm::mat4> boneTransforms;

		std::vector<SuspensionWheel> suspensionWheels;
		std::vector<SteeringWheel> steeringWheels;
		std::vector<SpinningWheel> spinningWheels;
	};
}
