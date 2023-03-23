#include <vob/aoe/ecs/world_data.h>


namespace vob::aoecs
{
	world_data::world_data(
		component_set a_worldComponents
		, component_list_factory const& a_componentListFactory
	)
		: m_worldComponents{ std::move(a_worldComponents) }
		, m_entityManager{ a_componentListFactory }
	{}

	void world_data::update()
	{
		m_oldEntityManager.update();
		m_entityManager.update();
	}
}