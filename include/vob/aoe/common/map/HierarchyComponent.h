#pragma once

#include <vob/aoe/ecs/_entity_id.h>

#include <vector>

namespace vob::aoe::common
{
	struct HierarchyComponent final
	{
		// Attributes
		_aoecs::entity_id m_parent;
		std::vector<_aoecs::entity_id> m_children;
	};
}

namespace vob::misvi
{
	template <typename VisitorType, typename ThisType>
	requires std::is_same_v<std::remove_cvref_t<ThisType>, aoe::common::HierarchyComponent>
	bool accept(VisitorType& a_visitor, ThisType& a_this)
	{
		return true;
	}
}
