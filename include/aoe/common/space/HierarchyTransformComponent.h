#pragma once

#include <aoe/core/ecs/Component.h>
#include <aoe/common/space/Vector.h>
#include <aoe/common/space/Quaternion.h>

namespace aoe
{
	namespace common
	{
		struct HierarchyTransformComponent final
			: public vis::Aggregate<HierarchyTransformComponent, ecs::AComponent>
		{
			// Attributes
			Vector3 m_oldPosition{};
			Quaternion m_oldRotation{1.0f, 0.0f, 0.0f, 0.0f};
			bool m_processed{ true };

			// Methods
			friend class vis::Aggregate<HierarchyTransformComponent, ecs::AComponent>;
			template <typename VisitorType, typename ThisType>
			// ReSharper disable once CppMemberFunctionMayBeStatic
			static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
			{
			}
		};
	}
}