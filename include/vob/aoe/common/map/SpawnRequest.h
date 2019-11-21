#pragma once

#include <vob/aoe/core/data/Handle.h>
#include <vob/aoe/core/standard/ADynamicType.h>
#include <vob/aoe/core/ecs/ComponentManager.h>

namespace vob::aoe::common
{
	struct SpawnRequest final
		: public type::ADynamicType
	{
		// Attributes
		data::Handle<ecs::ComponentManager> m_archetype;
		ecs::ComponentManager m_overrides;

		// Constructor
		explicit SpawnRequest(data::ADatabase& a_database
			, sta::Allocator<std::byte>& a_allocator)
			: m_archetype{ a_database }
			, m_overrides{ a_allocator }
		{}

		// Methods
		template <typename VisitorType>
		void accept(VisitorType& a_visitor)
		{
			a_visitor.visit(vis::makeNameValuePair("Archetype", m_archetype));
			a_visitor.visit(vis::makeNameValuePair("Overrides", m_overrides));
		}
	};
}
