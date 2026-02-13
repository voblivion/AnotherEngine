#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>

#include <vob/aoe/spacetime/Transform.h>
#include <vob/aoe/spacetime/InterpolatedTransform.h>
#include <vob/aoe/spacetime/TimeContext.h>
#include <vob/aoe/spacetime/InterpolationTimeComponent.h>
#include <vob/aoe/spacetime/InterpolationContext.h>

#include <chrono>


namespace vob::aoest
{
	class VOB_AOE_API TransformInterpolationSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoest::TimeContext> m_timeContext;
		aoeng::EcsWorldContextRef<aoest::InterpolationContext> m_interpolationContext;
		aoeng::EcsWorldViewRef<aoest::Position, aoest::Rotation, aoest::InterpolatedPosition, aoest::InterpolatedRotation, aoest::InterpolationTimeComponent> m_transformEntities;
	};
}
