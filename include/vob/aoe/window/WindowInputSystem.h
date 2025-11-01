#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/window/WindowContext.h>
#include <vob/aoe/window/WindowInputContext.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/input/Inputs.h>


namespace vob::aoewi
{
	class VOB_AOE_API WindowInputSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<WindowContext> m_windowContext;
		aoeng::EcsWorldContextRef<WindowInputContext> m_windowInputContext;
		aoeng::EcsWorldContextRef<aoein::Inputs> m_inputs;
	};
}
