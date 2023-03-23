#include <vob/aoe/ecs/_entity.h>


namespace vob::_aoecs
{
	// Public
	entity::entity(entity_id const a_id, component_manager a_componentManager)
		: component_manager{ std::move(a_componentManager) }
		, m_id { a_id }
	{}

	entity_id entity::get_id() const
	{
		return m_id;
	}
}