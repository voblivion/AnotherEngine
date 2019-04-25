#include <aoe/core/ecs/WorldData.h>


namespace aoe
{
	namespace ecs
	{
		WorldData::WorldData(ComponentManager a_worldComponents)
			: m_worldComponents{ std::move(a_worldComponents) }
			, m_entityManager{ m_worldComponents.getAllocator() }
		{}

		void WorldData::update()
		{
			m_entityManager.update();
		}

		sta::Allocator<std::byte> WorldData::getAllocator() const
		{
			return m_worldComponents.getAllocator();
		}
	}
}