#pragma once

#include <vob/aoe/input/binding_system.h>


namespace vob::aoein
{
	binding_system::binding_system(aoeng::world_data_provider& a_wdp)
		: m_bindings{ a_wdp }
		, m_inputs{ a_wdp }
		, m_presentationTimeWorldComponent{ a_wdp }
	{}

	void binding_system::update() const
	{
		m_bindings->update(*m_inputs, m_presentationTimeWorldComponent->m_elapsedTime);
	}
}
