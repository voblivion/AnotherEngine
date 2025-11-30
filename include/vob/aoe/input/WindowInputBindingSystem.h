#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/input/GameInputBindingContext.h>
#include <vob/aoe/input/GameInputContext.h>
#include <vob/aoe/spacetime/TimeContext.h>
#include <vob/aoe/window/WindowContext.h>



namespace vob::aoein
{
	struct VOB_AOE_API WindowInputBindingSystem
	{
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
		{
			m_timeContext.init(a_wdar);
			m_windowContext.init(a_wdar);
			m_gameInputContext.init(a_wdar);
			m_inputBindingContext.init(a_wdar);
		}

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoest::TimeContext> m_timeContext;
		aoeng::EcsWorldContextRef<aoewi::WindowContext> m_windowContext;
		aoeng::EcsWorldContextRef<GameInputBindingContext> m_inputBindingContext;
		aoeng::EcsWorldContextRef<GameInputContext> m_gameInputContext;
	};
}
