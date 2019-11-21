#pragma once

#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/core/ecs/EntityHandle.h>

namespace vob::aoe::common
{
	struct HierarchyComponent final
		: public vis::Aggregate<HierarchyComponent, ecs::AComponent>
	{
		// Attributes
		ecs::EntityHandle m_parent;
		std::pmr::vector<ecs::EntityHandle> m_children;

		// Methods
		friend class vis::Aggregate<HierarchyComponent, ecs::AComponent>;
		template <typename VisitorType, typename ThisType>
		// ReSharper disable once CppMemberFunctionMayBeStatic
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
		}
	};
}
