#include <vob/aoe/engine/world_data.h>


namespace vob::aoeng
{
	world_data::world_data(
		registry_allocator const& a_registryAllocator,
		registry_query_allocator const& a_entityRegistryQueryAllocator
	)
		: m_registry{ a_registryAllocator }
		, m_pendingEntityRegistryQueries{ a_entityRegistryQueryAllocator }
	{
	}

	void world_data::process_pending_registry_queries()
	{
		for (auto& entityRegistryQuery : m_pendingEntityRegistryQueries)
		{
			entityRegistryQuery(m_registry);
		}
		m_pendingEntityRegistryQueries.clear();
	}
}
