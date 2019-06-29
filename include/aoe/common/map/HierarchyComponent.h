#pragma once

#include <aoe/core/ecs/EntityId.h>
#include <aoe/core/ecs/Component.h>

namespace aoe
{
	namespace common
	{
		struct HierarchyComponent final
			: public ecs::ComponentDefaultImpl<HierarchyComponent>
		{
			// Attributes
			std::pmr::vector<ecs::EntityId> m_children;

			// Methods
			template <typename VisitorType>
			// ReSharper disable once CppMemberFunctionMayBeStatic
			void accept(VisitorType& a_visitor)
			{
			}
		};
	}
}
