#pragma once

#include "vob/aoe/debug/DebugUiContext.h"

#include "vob/aoe/engine/EcsWorldDataAccess.h"
#include "vob/aoe/input/GameInputContext.h"


namespace vob::aoedb
{
	class DebugUiSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<DebugUiContext> m_debugUiCtx;
		aoeng::EcsWorldContextRef<aoein::GameInputContext const> m_gameInputCtx;
	};
}
