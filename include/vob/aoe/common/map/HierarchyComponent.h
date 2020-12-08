#pragma once

#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/core/ecs/EntityHandle.h>
#include <vob/aoe/core/visitor/Traits.h>

namespace vob::aoe::common
{
	struct HierarchyComponent final
		: public ecs::AComponent
	{
		// Attributes
		ecs::EntityHandle m_parent;
		std::vector<ecs::EntityHandle> m_children;
	};
}

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::HierarchyComponent, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
	}
}
