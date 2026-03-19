#include <vob/aoe/spacetime/TransformInterpolationSystem.h>

#include <vob/misc/std/container_util.h>


namespace vob::aoest
{
	void TransformInterpolationSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{

	}

	void TransformInterpolationSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto const& timeContext = m_timeContext.get(a_wdap);
		auto const& interpolationContext = m_interpolationContext.get(a_wdap);

		for (auto [entity, position, rotation, interpolatedPosition, interpolatedRotation, interpolatedTime] : m_transformEntities.get(a_wdap).each())
		{
			auto const currentTime = timeContext.tickStartTime - interpolationContext.offset;
			auto sourceOffset = 0;
			while (sourceOffset + 1 < interpolatedTime.times.size()
				&& interpolatedTime.times[(interpolatedTime.endIndex + sourceOffset + 1) % interpolatedTime.times.size()] < currentTime)
			{
				++sourceOffset;
			}

			if (sourceOffset + 1 == mistd::isize(interpolatedTime.times))
			{
				position = interpolatedPosition.positions[(interpolatedPosition.endIndex + sourceOffset) % interpolatedTime.times.size()];
				continue;
			}

			auto const targetOffset = sourceOffset + 1;
			auto const sourceTime = interpolatedTime.times[(interpolatedTime.endIndex + sourceOffset) % interpolatedTime.times.size()];
			auto const targetTime = interpolatedTime.times[(interpolatedTime.endIndex + targetOffset) % interpolatedTime.times.size()];
			auto const sourcePosition = interpolatedPosition.positions[(interpolatedPosition.endIndex + sourceOffset) % interpolatedTime.times.size()];
			auto const targetPosition = interpolatedPosition.positions[(interpolatedPosition.endIndex + targetOffset) % interpolatedTime.times.size()];

			auto const sourceRotation = interpolatedRotation.rotations[(interpolatedRotation.endIndex + sourceOffset) % interpolatedTime.times.size()];
			auto const targetRotation = interpolatedRotation.rotations[(interpolatedRotation.endIndex + targetOffset) % interpolatedTime.times.size()];


			auto const progressRatio = std::chrono::duration<float>(currentTime - sourceTime).count()
				/ std::chrono::duration<float>(targetTime - sourceTime).count();

			position = sourcePosition + progressRatio * (targetPosition - sourcePosition);
			rotation = glm::slerp(sourceRotation, targetRotation, progressRatio);
		}
	}
}
