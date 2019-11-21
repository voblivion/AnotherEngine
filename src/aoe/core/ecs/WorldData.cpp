#include <vob/aoe/core/ecs/WorldData.h>


namespace vob::aoe::ecs
{
	WorldData::WorldData(ComponentManager a_worldComponents)
		: m_worldComponents{ std::move(a_worldComponents) }
		, m_entityManager{ m_worldComponents.getAllocator() }
	{}

	void WorldData::update()
	{
		m_entityManager.update();
	}

	std::pmr::polymorphic_allocator<std::byte> WorldData::getAllocator() const
	{
		return m_worldComponents.getAllocator();
	}
}