#pragma once

#include <aoe/core/Export.h>
#include <aoe/core/ecs/ComponentManager.h>
#include <aoe/core/ecs/EntityId.h>
#include "aoe/common/time/TimeComponent.h"

namespace aoe
{
	namespace ecs
	{
		class Entity
			: public ComponentManager
		{
		public:
			// Constructors
			AOE_CORE_API explicit Entity(EntityId const a_id
				, ComponentManager a_componentManager);

			// Methods
			AOE_CORE_API EntityId getId() const;

		private:
			// Attributes
			EntityId const m_id;
		};
	}
}
