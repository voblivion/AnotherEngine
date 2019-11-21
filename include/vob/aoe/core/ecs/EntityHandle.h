#pragma once

#include <vob/aoe/core/ecs/EntityId.h>
#include <vob/aoe/core/ecs/Entity.h>
#include <vob/aoe/core/ecs/SystemEntity.h>

namespace vob::aoe::ecs
{
	struct EntityHandle
	{
		// Attributes
		EntityId m_id = g_invalidEntityId;

		// Constructors
		EntityHandle() = default;
		
		explicit EntityHandle(EntityId const a_id)
			: m_id{ a_id }
		{}
		
		explicit EntityHandle(ecs::Entity const& a_entity)
			: m_id{ a_entity.getId() }
		{}

		template <typename... ComponentTypes>
		explicit EntityHandle(
			SystemEntity<ComponentTypes...> const& a_entity
		)
			: m_id{ a_entity.getId() }
		{}

		// Methods
		bool isValid() const
		{
			return m_id != g_invalidEntityId;
		}

		void reset()
		{
			m_id = g_invalidEntityId;
		}
	};
}
