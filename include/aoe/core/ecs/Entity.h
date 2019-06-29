#pragma once

#include <aoe/core/Export.h>
#include <aoe/core/ecs/ComponentManager.h>
#include <aoe/core/ecs/EntityId.h>

namespace aoe
{
	namespace ecs
	{
		class Entity final
		: public ComponentManager
		{
		public:
			// Constructors
			AOE_CORE_API explicit Entity(EntityId const a_id
				, ComponentManager const& a_componentManager);

			// Methods
			AOE_CORE_API EntityId getId() const;

		private:
			// Attributes
			EntityId const m_id;
		};
	}
}
