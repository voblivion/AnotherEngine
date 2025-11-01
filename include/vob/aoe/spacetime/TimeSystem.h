#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/spacetime/TimeContext.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>


namespace vob::aoest
{
	class VOB_AOE_API TimeSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<TimeContext> m_timeContext;
	};
}
