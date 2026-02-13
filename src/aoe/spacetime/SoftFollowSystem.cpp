#include <vob/aoe/spacetime/SoftFollowSystem.h>

#include <vob/aoe/spacetime/Transform.h>

#include <glm/gtc/quaternion.hpp>

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

		for (auto [entity, position, rotation, softFollowComponent] : m_softFollowingEntities.get(a_wdap).each())
		{
			if (!m_softFollowableEntities.get(a_wdap).contains(softFollowComponent.target))
			{
				continue;
			}

			auto const& [followedPosition, followedRotation] = m_softFollowableEntities.get(a_wdap).get(softFollowComponent.target);

			auto const targetPosition = followedPosition + followedRotation * softFollowComponent.positionOffset;
			if (glm::distance(position, targetPosition) > std::numeric_limits<float>::epsilon())
			{
				auto const toTargetPosition = targetPosition - position;
				auto const force = softFollowComponent.elasticity * toTargetPosition - softFollowComponent.damping * softFollowComponent.velocity;
				position += softFollowComponent.velocity * (elapsedTime / 2);
				softFollowComponent.velocity += force / softFollowComponent.mass * elapsedTime;
				position += softFollowComponent.velocity * (elapsedTime / 2);
			}

			// TODO: meh figure why math wrong
			position = targetPosition;

			auto const targetAim = followedPosition + followedRotation * softFollowComponent.aimOffset;
			auto const toTargetAim = targetAim - position;
			auto const aimDir = glm::normalize(
				glm::length(toTargetAim) > std::numeric_limits<float>::epsilon() ? toTargetAim : followedRotation * glm::vec3{ 0.0f, 0.0f, -1.0f });
			rotation = glm::quatLookAt(aimDir, glm::vec3{ 0.0f, 1.0f, 0.0f });
		}
	}
}
