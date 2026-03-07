#include <vob/aoe/rendering/CarRigSystem.h>

#include <glm/ext/matrix_transform.hpp>

#include <cmath>
#include <numbers>

#pragma optimize("", off)
namespace vob::aoegl
{
	void CarRigSystem::init(vob::aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{

	}

	void CarRigSystem::execute(vob::aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto const elapsedTime = std::chrono::duration<float>(m_timeContext.get(a_wdap).elapsedTime).count();

		auto const carEntities = m_carEntities.get(a_wdap);
		for (auto [entity, rotation, carColliderCmp, carControllerCmp, carRigCmp, riggedModelCmp] : carEntities.each())
		{
			auto const forward = rotation * glm::vec3{ 0.0f, 0.0f, -1.0f };

			for (const auto& suspensionWheel : carRigCmp.suspensionWheels)
			{
				auto const& wheelState = carColliderCmp.wheels[suspensionWheel.wheelIndex];
    			auto const suspensionTransform = glm::translate(glm::mat4(1.0f), glm::vec3{ 0.0f, -wheelState.suspensionLength, 0.0f });
				auto const& parentBoneTransform = carRigCmp.boneTransforms[suspensionWheel.parentBoneIndex];
				carRigCmp.boneTransforms[suspensionWheel.boneIndex] = parentBoneTransform * suspensionWheel.baseTransformRelativeParent * suspensionTransform;
				carRigCmp.bones[suspensionWheel.boneIndex] = carRigCmp.boneTransforms[suspensionWheel.boneIndex] * suspensionWheel.invBasePose;
			}

			for (const auto& steeringWheel : carRigCmp.steeringWheels)
			{
				auto const& wheelState = carControllerCmp.wheels[steeringWheel.wheelIndex];
				auto const steerTransform = glm::rotate(glm::mat4(1.0f), wheelState.steeringAngle, glm::vec3{ 0.0f, 0.0f, 1.0f });
				auto const& parentBoneTransform = carRigCmp.boneTransforms[steeringWheel.parentBoneIndex];
				carRigCmp.boneTransforms[steeringWheel.boneIndex] = parentBoneTransform * steeringWheel.baseTransformRelativeParent * steerTransform;
				carRigCmp.bones[steeringWheel.boneIndex] = carRigCmp.boneTransforms[steeringWheel.boneIndex] * steeringWheel.invBasePose;
			}

			for (auto& spinningWheel : carRigCmp.spinningWheels)
			{
				spinningWheel.distance += glm::dot(carColliderCmp.linearVelocity, forward) * elapsedTime;
				spinningWheel.distance = std::fmod(spinningWheel.distance, 2.0f * std::numbers::pi_v<float> *spinningWheel.wheelRadius);
				auto const spinTransform = glm::rotate(
					glm::mat4(1.0f), spinningWheel.distance / spinningWheel.wheelRadius, glm::vec3{ 0.0f, 1.0f, 0.0f });
				auto const& parentBoneTransform = carRigCmp.boneTransforms[spinningWheel.parentBoneIndex];
				carRigCmp.boneTransforms[spinningWheel.boneIndex] = parentBoneTransform * spinningWheel.baseTransformRelativeParent * spinTransform;
				carRigCmp.bones[spinningWheel.boneIndex] = carRigCmp.boneTransforms[spinningWheel.boneIndex] * spinningWheel.invBasePose;
			}

			glNamedBufferSubData(
				riggedModelCmp.rigParamsUbo, 0, carRigCmp.bones.size() * sizeof(glm::mat4), carRigCmp.bones.data());
		}
	}
}
