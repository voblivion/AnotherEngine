#include <vob/aoe/core/ecs/WorldData.h>


namespace vob::aoe::ecs
{
	WorldData::WorldData(ComponentManager a_worldComponents)
		: m_worldComponents{ std::move(a_worldComponents) }
	{}

	void WorldData::update()
	{
		m_entityManager.update();
	}
}