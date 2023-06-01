#include <vob/aoe/engine/world_data.h>


namespace vob::aoeng
{
	world_data::world_data(
		entity_allocator const& a_entityAllocator,
		entity_registry_query_allocator const& a_entityRegistryQueryAllocator
	)
		: m_entityRegistry{ a_entityAllocator }
		, m_worldEntity{ m_entityRegistry.create() }
		, m_pendingEntityRegistryQueries{ a_entityRegistryQueryAllocator }
	{
	}

	void world_data::process_pending_registry_queries()
	{
		for (auto& entityRegistryQuery : m_pendingEntityRegistryQueries)
		{
			entityRegistryQuery(m_entityRegistry);
		}
		m_pendingEntityRegistryQueries.clear();
	}
}
