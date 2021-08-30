#pragma once

// #include <vob/aoe/core/standard/ADynamicType.h>
#include <vob/aoe/core/ecs/ComponentManager.h>

/*namespace vob::aoe::common
{
	struct SpawnRequest final
		: public type::ADynamicType
	{
		// Attributes
		std::shared_ptr<ecs::ComponentManager const> m_archetype;
		ecs::ComponentManager m_overrides;

		// Constructor
		explicit SpawnRequest(sta::Allocator<std::byte>& a_allocator)
			: m_overrides{ a_allocator }
		{}

		// Methods
		template <typename VisitorType>
		void accept(VisitorType& a_visitor)
		{
			a_visitor.visit(vis::makeNameValuePair("Archetype", m_archetype));
			a_visitor.visit(vis::makeNameValuePair("Overrides", m_overrides));
		}
	};
}*/

