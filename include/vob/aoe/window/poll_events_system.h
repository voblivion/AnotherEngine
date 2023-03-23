#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/window/window_world_component.h>

#include <vob/aoe/ecs/world_data_provider.h>


namespace vob::aoewi
{
	class VOB_AOE_API poll_events_system
	{
	public:
		explicit poll_events_system(aoecs::world_data_provider& a_wdp);

		void update() const;

	private:
		window_world_component& m_windowWorldComponent;
	};
}
