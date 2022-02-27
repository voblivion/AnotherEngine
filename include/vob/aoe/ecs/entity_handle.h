#pragma once

#include <vob/aoe/ecs/entity_id.h>
#include <vob/aoe/ecs/entity.h>
#include <vob/aoe/ecs/entity_view.h>

namespace vob::aoecs
{
#pragma warning("Should be removed, use entity_id instead?")
	struct entity_handle
	{
		// Attributes
		entity_id m_id = k_invalid_entity_id;

		// Constructors
		entity_handle() = default;
		
		explicit entity_handle(entity_id const a_id)
			: m_id{ a_id }
		{}
		
		explicit entity_handle(aoecs::entity const& a_entity)
			: m_id{ a_entity.get_id() }
		{}

		template <typename... ComponentTypes>
		explicit entity_handle(
			EntityView<ComponentTypes...> const& a_entity
		)
			: m_id{ a_entity.getId() }
		{}

		// Operators
		friend bool operator==(entity_handle const& a_lhs, entity_handle const& a_rhs)
		{
			return a_lhs.m_id == a_rhs.m_id;
		}

		// Methods
		bool is_valid() const
		{
			return m_id != k_invalid_entity_id;
		}

		void reset()
		{
			m_id = k_invalid_entity_id;
		}
	};
}
