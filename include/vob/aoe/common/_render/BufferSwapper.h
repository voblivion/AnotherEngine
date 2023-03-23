#pragma once

#include <vob/aoe/ecs/world_data_provider.h>

#include <vob/aoe/common/window/WorldWindowcomponent.h>


namespace vob::aoe::common
{
	class BufferSwapper
	{
	public:
		// Constructor
		explicit BufferSwapper(aoecs::world_data_provider& a_wdp)
			: m_worldWindowComponent{ a_wdp.get_world_component<WorldWindowComponent>() }
		{}

		// Methods
		void run() const
		{
			m_worldWindowComponent.getWindow().swapBuffer();
		}

	private:
		// Attributes
		WorldWindowComponent& m_worldWindowComponent;
	};
}
