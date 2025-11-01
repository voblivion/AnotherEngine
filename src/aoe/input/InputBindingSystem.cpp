#include <vob/aoe/input/InputBindingSystem.h>


namespace vob::aoein
{
	void InputBindingSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_inputs.init(a_wdar);
		m_timeContext.init(a_wdar);
		m_inputBindings.init(a_wdar);
	}

	void InputBindingSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		m_inputBindings.get(a_wdap).update(m_inputs.get(a_wdap), m_timeContext.get(a_wdap).elapsedTime);
	}
}
