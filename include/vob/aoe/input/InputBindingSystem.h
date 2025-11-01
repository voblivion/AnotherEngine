#pragma once

#include <vob/aoe/input/InputBindings.h>
#include <vob/aoe/input/Inputs.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/spacetime/TimeContext.h>


namespace vob::aoein
{
	class VOB_AOE_API InputBindingSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);
		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<Inputs> m_inputs;
		aoeng::EcsWorldContextRef<aoest::TimeContext const> m_timeContext;
		aoeng::EcsWorldContextRef<InputBindings> m_inputBindings;
	};
}
