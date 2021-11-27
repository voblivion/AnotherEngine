#pragma once

#include <vob/aoe/api.h>
#include <vob/aoe/ecs/ComponentManager.h>
#include <vob/aoe/ecs/EntityId.h>

namespace vob::aoecs
{
	class Entity final
		: public ComponentManager
	{
	public:
		// Constructors
		VOB_AOE_API explicit Entity(
			EntityId const a_id
			, ComponentManager a_componentManager
		);

		// Methods
		VOB_AOE_API EntityId getId() const;

	private:
		// Attributes
		EntityId const m_id;
	};
}
