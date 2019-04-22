#pragma once

#include <aoe/Config.h>
#include <aoe/core/ComponentManager.h>
#include <aoe/core/EntityId.h>

namespace aoe
{
	namespace core
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
