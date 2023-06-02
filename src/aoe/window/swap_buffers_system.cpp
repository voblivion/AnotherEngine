#include <vob/aoe/window/swap_buffers_system.h>


namespace vob::aoewi
{
	swap_buffers_system::swap_buffers_system(aoeng::world_data_provider& a_wdp)
		: m_windowWorldComponent{ a_wdp }
	{}

	void swap_buffers_system::update() const
	{
		m_windowWorldComponent->m_window.get().swap_buffers();
	}
}
