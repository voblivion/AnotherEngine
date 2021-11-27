#pragma once

#include <vob/aoe/ecs/Component.h>
#include <vob/aoe/ecs/EntityHandle.h>
#include <vob/aoe/core/visitor/Traits.h>

namespace vob::aoe::common
{
	struct HierarchyComponent final
		: public aoecs::AComponent
	{
		// Attributes
		aoecs::EntityHandle m_parent;
		std::vector<aoecs::EntityHandle> m_children;
	};
}

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::HierarchyComponent, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
	}
}
