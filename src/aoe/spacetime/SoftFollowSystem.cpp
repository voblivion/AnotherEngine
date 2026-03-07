#include <vob/aoe/spacetime/SoftFollowSystem.h>

#include <vob/aoe/spacetime/Transform.h>

#include <glm/gtc/quaternion.hpp>

#include "imgui.h"

#include <chrono>
#include <limits>


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
		if (ImGui::Begin("Soft Follow"))
		{
			ImGui::Checkbox("Display Debug", &k_debugSoftFollow);
			ImGui::End();
		}

		for (auto [entity, position, rotation, softFollowComponent] : m_softFollowingEntities.get(a_wdap).each())
		{
			if (!m_softFollowableEntities.get(a_wdap).contains(softFollowComponent.target))
			{
				continue;
			}

			auto const& [followedPosition, followedRotation] = m_softFollowableEntities.get(a_wdap).get(softFollowComponent.target);

			static float k_slowSpeed = 1.0f;
			static float k_fastSpeed = 20.0f;

			auto const slowPosition = followedPosition + followedRotation * softFollowComponent.positionOffset;

			auto fastPosition = slowPosition;
			auto const speed = glm::length(followedPosition - softFollowComponent.prevTargetPosition) / elapsedTime;
			if (speed > k_slowSpeed)
			{
				glm::vec3 velocityDir = glm::normalize(followedPosition - softFollowComponent.prevTargetPosition);
				glm::vec3 rightDir = glm::normalize(glm::cross(glm::vec3{ 0.0f, 1.0f, 0.0f }, -velocityDir));
				glm::vec3 upDir = glm::cross(-velocityDir, rightDir);
				glm::mat3 local(rightDir, upDir, -velocityDir);
				fastPosition = followedPosition + local * softFollowComponent.positionOffset;
			}

			auto const targetPosition = slowPosition + std::clamp((speed - k_slowSpeed) / (k_fastSpeed - k_slowSpeed), 0.0f, 1.0f) * (fastPosition - slowPosition);

			if (glm::distance(position, targetPosition) > std::numeric_limits<float>::epsilon())
			{
				auto const toTargetPosition = targetPosition - position;
				auto const force = softFollowComponent.elasticity * toTargetPosition - softFollowComponent.damping * softFollowComponent.velocity;
				position += softFollowComponent.velocity * (elapsedTime / 2);
				softFollowComponent.velocity += force / softFollowComponent.mass * elapsedTime;
				position += softFollowComponent.velocity * (elapsedTime / 2);
			}

			softFollowComponent.prevTargetPosition = followedPosition;

			if (k_debugSoftFollow)
			{
				m_debugMeshContext.get(a_wdap).addSphere(slowPosition, 0.1f, aoegl::k_blue);
				m_debugMeshContext.get(a_wdap).addSphere(targetPosition, 0.15f, aoegl::k_green);
				m_debugMeshContext.get(a_wdap).addSphere(fastPosition, 0.1f, aoegl::k_red);
				m_debugMeshContext.get(a_wdap).addLine(slowPosition, targetPosition, aoegl::k_blue);
				m_debugMeshContext.get(a_wdap).addLine(targetPosition, fastPosition, aoegl::k_red);
			}

			auto const targetAim = followedPosition + followedRotation * softFollowComponent.aimOffset;
			auto const toTargetAim = targetAim - position;
			auto const aimDir = glm::normalize(
				glm::length(toTargetAim) > std::numeric_limits<float>::epsilon() ? toTargetAim : followedRotation * glm::vec3{ 0.0f, 0.0f, -1.0f });
			rotation = glm::quatLookAt(aimDir, glm::vec3{ 0.0f, 1.0f, 0.0f });
		}
	}
}
