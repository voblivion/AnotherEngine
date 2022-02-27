#include <vob/aoe/ecs/WorldData.h>


namespace vob::aoecs
{
	WorldData::WorldData(component_manager a_worldComponents)
		: m_worldComponents{ std::move(a_worldComponents) }
	{}

	void WorldData::update()
	{
		m_entityManager.update();
	}
}