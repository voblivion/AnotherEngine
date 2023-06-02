#include <vob/aoe/engine/world.h>

#include <vob/misc/std/message_macros.h>

#pragma message(VOB_MISTD_TODO "max framerate should be handled by a system.")
#include <chrono>
#include <thread>


namespace vob::aoeng
{
	world::world(
		entity_allocator const& a_entityAllocator,
		entity_registry_query_allocator const& a_entityRegistryQueryAllocator,
		system_allocator const& a_systemAllocator
	)
		: m_worldData{ a_entityAllocator, a_entityRegistryQueryAllocator }
	{
	}

	void world::start(mismt::pmr::schedule a_schedule)
	{
		mismt::pmr::worker systemWorker{ m_systemTasks, std::move(a_schedule) };
		while (!m_worldData.should_stop())
		{
			systemWorker.execute();
			m_worldData.process_pending_registry_queries();

#pragma message(VOB_MISTD_TODO "max framerate should be handled by a system.")
			std::this_thread::sleep_for(std::chrono::milliseconds(0));
		}
	}
}
