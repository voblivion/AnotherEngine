#pragma once

#include <aoe/core/ecs/Component.h>
#include <aoe/common/space/Vector.h>
#include <aoe/common/space/Quaternion.h>

namespace aoe
{
	namespace common
	{
		struct HierarchyTransformComponent final
			: public ecs::ComponentDefaultImpl<HierarchyTransformComponent>
		{
			// Attributes
			Vector3 m_oldPosition{};
			Quaternion m_oldRotation{1.0f, 0.0f, 0.0f, 0.0f};
			bool m_processed{ true };

			// Methods
			template <typename VisitorType>
			// ReSharper disable once CppMemberFunctionMayBeStatic
			void accept(VisitorType& a_visitor)
			{
			}
		};
	}
}