#pragma once

#include <vob/aoe/ecs/WorldDataProvider.h>

#include <vob/aoe/common/window/WorldWindowComponent.h>


namespace vob::aoe::common
{
	class BufferSwapper
	{
	public:
		// Constructor
		explicit BufferSwapper(aoecs::WorldDataProvider& a_wdp)
			: m_worldWindowComponent{ a_wdp.getWorldComponentRef<WorldWindowComponent>() }
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
