#pragma once

#include <vob/aoe/ecs/entity_map.h>
#include <vob/aoe/ecs/world_data_provider.h>


namespace vob::aoecs
{
	template <typename... TComponents>
	class entity_map_observer_list_ref
	{
	public:
		explicit entity_map_observer_list_ref(world_data_provider& a_wdp)
			: m_entities{ a_wdp.observe_entities<TComponents...>() }
		{}

		bool empty() const
		{
			return m_entities.empty();
		}

		auto begin() const
		{
			return m_entities.begin();
		}

		auto end() const
		{
			return m_entities.end();
		}

		auto find(entity_id const a_id) const
		{
			return m_entities.find(a_id);
		}

		auto const& operator*() const
		{
			return m_entities;
		}

	private:
		entity_map_observer_list<TComponents...> const& m_entities;
	};

}
