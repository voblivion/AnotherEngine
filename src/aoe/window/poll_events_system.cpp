#include <vob/aoe/window/poll_events_system.h>


namespace vob::aoewi
{
	poll_events_system::poll_events_system(aoecs::world_data_provider& a_wdp)
		: m_windowWorldComponent{ a_wdp.get_world_component<window_world_component>() }
	{}

	void poll_events_system::update() const
	{
		m_windowWorldComponent.m_window.get().poll_events();
	}
}
