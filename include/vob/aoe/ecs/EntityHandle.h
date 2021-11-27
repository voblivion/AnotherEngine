#pragma once

#include <vob/aoe/ecs/EntityId.h>
#include <vob/aoe/ecs/Entity.h>
#include <vob/aoe/ecs/EntityView.h>

namespace vob::aoe::aoecs
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
		
		explicit EntityHandle(aoecs::Entity const& a_entity)
			: m_id{ a_entity.getId() }
		{}

		template <typename... ComponentTypes>
		explicit EntityHandle(
			EntityView<ComponentTypes...> const& a_entity
		)
			: m_id{ a_entity.getId() }
		{}

		// Operators
		friend bool operator==(EntityHandle const& a_lhs, EntityHandle const& a_rhs)
		{
			return a_lhs.m_id == a_rhs.m_id;
		}

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
