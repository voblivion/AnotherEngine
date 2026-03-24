#include <vob/aoe/rendering/CarRigSystem.h>

#include <glm/ext/matrix_transform.hpp>

#include <cmath>
#include <numbers>


namespace vob::aoegl
{
	void CarRigSystem::init(vob::aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{

	}

	void CarRigSystem::execute(vob::aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto const elapsedTime = std::chrono::duration<float>(m_timeContext.get(a_wdap).elapsedTime).count();

		auto const carEntities = m_carEntities.get(a_wdap);
		for (auto [entity, rotation, linearVelocityCmp, carColliderCmp, carControllerCmp, riggedModelCmp, carRigCmp] : carEntities.each())
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
				auto const steerTransform = glm::rotate(glm::mat4(1.0f), carControllerCmp.state.steering * 0.52f, glm::vec3{ 0.0f, 0.0f, 1.0f });
				auto const& parentBoneTransform = carRigCmp.boneTransforms[steeringWheel.parentBoneIndex];
				carRigCmp.boneTransforms[steeringWheel.boneIndex] = parentBoneTransform * steeringWheel.baseTransformRelativeParent * steerTransform;
				carRigCmp.bones[steeringWheel.boneIndex] = carRigCmp.boneTransforms[steeringWheel.boneIndex] * steeringWheel.invBasePose;
			}

			for (auto& spinningWheel : carRigCmp.spinningWheels)
			{
				auto const& wheelCol = carColliderCmp.wheels[spinningWheel.wheelIndex];
				// TODO: approximate .. need to account for suspension dir too .. whatever.
				auto const radius = glm::dot(wheelCol.radiuses, rotation * glm::vec3{ 0.0f, 1.0f, 0.0f });
				auto const spin = carControllerCmp.state.currentGearIndex != 0 ? 1.0f : -1.0f;
				spinningWheel.distance += spin * carControllerCmp.state.rotationSpeed * radius * elapsedTime;
				spinningWheel.distance = std::fmod(spinningWheel.distance, 2.0f * std::numbers::pi_v<float> *radius);
				auto const spinTransform = glm::rotate(
					glm::mat4(1.0f), spinningWheel.distance / radius, glm::vec3{ 0.0f, 1.0f, 0.0f });
				auto const& parentBoneTransform = carRigCmp.boneTransforms[spinningWheel.parentBoneIndex];
				carRigCmp.boneTransforms[spinningWheel.boneIndex] = parentBoneTransform * spinningWheel.baseTransformRelativeParent * spinTransform;
				carRigCmp.bones[spinningWheel.boneIndex] = carRigCmp.boneTransforms[spinningWheel.boneIndex] * spinningWheel.invBasePose;
			}

			glNamedBufferSubData(
				riggedModelCmp.rigParamsUbo, 0, carRigCmp.bones.size() * sizeof(glm::mat4), carRigCmp.bones.data());
		}
	}
}
