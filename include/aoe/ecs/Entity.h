#pragma once

#include <aoe/Config.h>
#include <aoe/ecs/ComponentManager.h>
#include <aoe/ecs/EntityId.h>

namespace aoe
{
	namespace ecs
	{
		class Entity : public ComponentManager
		{
		public:
			// Constructors
			AOE_API explicit Entity(EntityId const a_id
				, ComponentManager const& a_componentManager);

			// Methods
			AOE_API EntityId getId() const;

		private:
			// Attributes
			EntityId const m_id;
		};
	}
}
