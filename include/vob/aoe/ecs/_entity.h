#pragma once

#include <vob/aoe/api.h>
#include <vob/aoe/ecs/_component_manager.h>
#include <vob/aoe/ecs/_entity_id.h>

namespace vob::_aoecs
{
	class entity final
		: public component_manager
	{
	public:
		// Constructors
		VOB_AOE_API explicit entity(entity_id const a_id, component_manager a_componentManager);

		// Methods
		VOB_AOE_API entity_id get_id() const;

	private:
		// Attributes
		entity_id const m_id;
	};
}
