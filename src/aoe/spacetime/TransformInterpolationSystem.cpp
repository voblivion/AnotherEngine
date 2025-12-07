#include <vob/aoe/spacetime/TransformInterpolationSystem.h>


namespace vob::aoest
{
	void TransformInterpolationSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{

	}

	void TransformInterpolationSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto const& timeContext = m_timeContext.get(a_wdap);

		for (auto [entity, position, rotation, interpolatedPosition, interpolatedRotation, interpolationComponent] : m_transformEntities.get(a_wdap).each())
		{
			auto const sourceToTarget = std::chrono::duration<float>(interpolationComponent.targetTime - interpolationComponent.sourceTime).count();
			if (std::abs(sourceToTarget) < std::numeric_limits<float>::epsilon())
			{

				continue;
			}

			auto const sourceToNow = std::chrono::duration<float>(timeContext.tickStartTime - interpolationComponent.sourceTime).count();
			auto const alpha = std::clamp(sourceToNow / sourceToTarget, 0.0f, 1.0f);

			position = interpolatedPosition.source + alpha * (interpolatedPosition.target - interpolatedPosition.source);
			rotation = glm::slerp(interpolatedRotation.source, interpolatedRotation.target, alpha);
		}
	}
}
