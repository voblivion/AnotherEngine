#include <vob/aoe/ecs/world_data.h>


namespace vob::aoecs
{
	world_data::world_data(component_manager a_worldComponents)
		: m_worldComponents{ std::move(a_worldComponents) }
	{}

	void world_data::update()
	{
		m_entityManager.update();
	}
}