#pragma once

#include <vob/aoe/core/ecs/WorldDataProvider.h>

#include <vob/aoe/common/window/WindowComponent.h>


namespace vob::aoe::common
{
	class BufferSwapper
	{
	public:
		// Constructor
		explicit BufferSwapper(ecs::WorldDataProvider& a_wdp)
			: m_windowComponent{ a_wdp.getWorldComponentRef<WindowComponent>() }
		{}

		// Methods
		void run() const
		{
			m_windowComponent.getWindow().swapBuffer();
		}

	private:
		// Attributes
		WindowComponent& m_windowComponent;
	};
}
