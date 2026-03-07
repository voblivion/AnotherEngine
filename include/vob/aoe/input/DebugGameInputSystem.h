#pragma once

#include "vob/aoe/api.h"

#include "vob/aoe/engine/EcsWorldDataAccess.h"


namespace vob::aoein
{
	struct GameInputContext;
	struct DebugGameInputContext;

	class VOB_AOE_API DebugGameInputSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoein::GameInputContext const> m_gameInputContext;
		aoeng::EcsWorldContextRef<aoein::DebugGameInputContext> m_debugGameInputContext;
	};
}
