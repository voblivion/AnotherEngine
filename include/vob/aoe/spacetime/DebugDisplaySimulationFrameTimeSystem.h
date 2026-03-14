#pragma once

#include <vob/aoe/spacetime/DebugSimulationFrameTimeHistoryContext.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>


namespace vob::aoest
{
	class DebugDisplaySimulationFrameTimeSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<DebugSimulationFrameTimeHistoryContext> m_debugSimulationFrameTimeHistoryContext;
	};
}
