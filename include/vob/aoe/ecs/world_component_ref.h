#pragma once

#include <vob/aoe/ecs/world_data_provider.h>


namespace vob::aoecs
{
	template <typename TWorldComponent>
	class world_component_ref
	{
	public:
		explicit world_component_ref(world_data_provider& a_wdp)
			: m_worldComponent{ a_wdp.get_world_component<TWorldComponent>() }
		{}

		TWorldComponent* operator->() const
		{
			return &m_worldComponent;
		}

		TWorldComponent& operator*() const
		{
			return m_worldComponent;
		}

	private:
		TWorldComponent& m_worldComponent;
	};
}
