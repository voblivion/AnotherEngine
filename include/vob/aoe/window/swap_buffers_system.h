#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/window/window_world_component.h>

#include <vob/aoe/ecs/world_data_provider.h>


namespace vob::aoewi
{
	class VOB_AOE_API swap_buffers_system
	{
	public:
		explicit swap_buffers_system(aoecs::world_data_provider& a_wdp);

		void update() const;

	private:
		window_world_component& m_windowWorldComponent;
	};
}
