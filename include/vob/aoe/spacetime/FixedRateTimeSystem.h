#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/spacetime/TimeContext.h>
#include <vob/aoe/spacetime/FixedRateTimeContext.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>


namespace vob::aoest
{
	class VOB_AOE_API FixedRateTimeSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<TimeContext> m_timeContext;
		aoeng::EcsWorldContextRef<FixedRateTimeContext> m_fixedRateTimeContext;
	};
}
