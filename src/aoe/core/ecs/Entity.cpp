#include <aoe/core/ecs/Entity.h>


namespace aoe
{
	namespace ecs
	{
		// Public
		Entity::Entity(EntityId const a_id
			, ComponentManager a_componentManager)
			: ComponentManager{ std::move(a_componentManager) }
			, m_id { a_id }
		{}

		EntityId Entity::getId() const
		{
			return m_id;
		}
	}
}