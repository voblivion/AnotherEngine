#pragma once

#include "vob/aoe/rendering/contexts/LimitFramesInFlightContext.h"

#include "vob/aoe/engine/EcsWorldDataAccess.h"


namespace vob::aoegl
{
	class LimitFramesInFlightSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<LimitFramesInFlightContext> m_limitFramesInFlightCtx;
	};
}
