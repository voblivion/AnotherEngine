#include <vob/aoe/spacetime/SoftFollowSystem.h>

#include "vob/aoe/spacetime/PositionComponent.h"
#include "vob/aoe/spacetime/RotationComponent.h"
#include "vob/aoe/spacetime/TransformUtils.h"

#include "vob/aoe/math/MathUtils.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "imgui.h"

#include <chrono>
#include <limits>


#pragma optimize("", off)
namespace vob::aoest
{
	void SoftFollowSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_timeContext.init(a_wdar);
		m_softFollowableEntities.init(a_wdar);
		m_softFollowingEntities.init(a_wdar);
	}

	void SoftFollowSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto const elapsedTime = std::chrono::duration<float>(m_timeContext.get(a_wdap).elapsedTime).count();

		static bool k_debugSoftFollow = false;
		auto const isImGuiOpen = ImGui::Begin("Soft Follow");
		if (isImGuiOpen)
		{
			ImGui::Checkbox("Display Debug", &k_debugSoftFollow);
		}

		static float k_minSpeedThreshold = 5.0f;
		static float k_maxSpeedThreshold = 20.0f;
		static float k_minPositionChangeRate = 0.1f;
		static float k_maxPositionChangeRate = 0.3f;
		static float k_minRotationChangeRate = 0.002f;
		static float k_maxRotationChangeRate = 0.02f;

		for (auto [entity, positionCmp, rotationCmp, softFollowCmp] : m_softFollowingEntities.get(a_wdap).each())
		{
			if (!m_softFollowableEntities.get(a_wdap).contains(softFollowCmp.target))
			{
				continue;
			}

			auto const& [followedPositionCmp, followedRotationCmp, followedLinearVelocityCmp] =
				m_softFollowableEntities.get(a_wdap).get(softFollowCmp.target);

			auto const& targetPosition = followedPositionCmp.value;
			auto const& targetRotation = followedRotationCmp.value;
			auto const& targetVelocity = followedLinearVelocityCmp.value;
			auto const targetSpeed = glm::length(targetVelocity);
			
			auto const movingBlend = std::clamp(targetSpeed / k_minSpeedThreshold, 0.0f, 1.0f);
			auto const speedBlend = std::clamp((targetSpeed - k_minSpeedThreshold) / (k_maxSpeedThreshold - k_minSpeedThreshold), 0.0f, 1.0f);

			// forward & up
			auto const targetForward = targetRotation * glm::vec3{ 0.0f, 0.0f, -1.0f };
			auto const velocityForward = aoema::normalizeWithDefault(targetVelocity, targetForward);
			auto const referenceForward = aoema::normalizeWithDefault(glm::mix(targetForward, velocityForward, movingBlend), velocityForward);
			auto const& referenceUp = softFollowCmp.referenceUp;

			// desired position
			auto const positionPitch = softFollowCmp.positionPitch;
			auto const behindDir = std::sin(positionPitch) * referenceUp - std::cos(positionPitch) * referenceForward;
			auto const desiredPosition = targetPosition + behindDir * softFollowCmp.positionDistance;

			// desired aim point
			auto const desiredAimPoint = targetPosition + referenceForward * softFollowCmp.lookAheadDistance;

			// smooth position
			auto const& currentDir = softFollowCmp.currentDir;
			auto const desiredDir = behindDir * softFollowCmp.positionDistance;
			auto const positionChangeRate = glm::mix(k_minPositionChangeRate * softFollowCmp.lookAheadDistance, k_maxPositionChangeRate * softFollowCmp.lookAheadDistance, speedBlend);
			auto const desiredPositionChange = desiredDir - currentDir;
			auto const newDir = glm::length(desiredPositionChange) < positionChangeRate * elapsedTime
				? desiredDir : currentDir + glm::normalize(desiredPositionChange) * positionChangeRate * elapsedTime;
			auto const newPosition = targetPosition + newDir;

			if (k_debugSoftFollow)
			{
				m_debugMeshContext.get(a_wdap).addSphere(positionCmp.value, 0.1f, aoegl::k_blue);
				m_debugMeshContext.get(a_wdap).addSphere(newPosition, 0.15f, aoegl::k_green);
				m_debugMeshContext.get(a_wdap).addSphere(desiredPosition, 0.1f, aoegl::k_red);
				m_debugMeshContext.get(a_wdap).addLine(positionCmp.value, newPosition, aoegl::k_green);
				m_debugMeshContext.get(a_wdap).addLine(newPosition, desiredPosition, aoegl::k_red);
			}

			positionCmp.value = newPosition;

			softFollowCmp.currentDir = newPosition - targetPosition;

			// smooth rotation
			auto const rotationChangeRate = glm::mix(k_minRotationChangeRate, k_maxRotationChangeRate, speedBlend);
			auto const toDesiredAimPoint = glm::normalize(desiredAimPoint - positionCmp.value);
			auto const desiredRotation = glm::quatLookAt(toDesiredAimPoint, referenceUp);
			rotationCmp.value = glm::slerp(rotationCmp.value, desiredRotation, rotationChangeRate);
		}

		ImGui::End();
	}
}
