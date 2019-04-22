#include <aoe/core/Entity.h>


namespace aoe
{
	namespace core
	{
		// Public
		Entity::Entity(EntityId const a_id,
			ComponentManager const& a_componentManager)
			: ComponentManager{ a_componentManager }
			, m_id{ a_id }
		{}

		EntityId Entity::getId() const
		{
			return m_id;
		}
	}
}