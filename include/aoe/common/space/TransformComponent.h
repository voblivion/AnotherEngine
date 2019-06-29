#pragma once

#include <aoe/core/ecs/Component.h>
#include <aoe/common/space/Vector.h>
#include <aoe/common/space/Quaternion.h>

namespace aoe
{
	namespace common
	{
		const Vector3 right = Vector3{ 1.0f, 0.0f, 0.0f };
		const Vector3 front = Vector3{ 0.0f, 1.0f, 0.0f };
		const Vector3 up = Vector3{ 0.0f, 0.0f, 1.0f };

		struct TransformComponent final
			: public ecs::ComponentDefaultImpl<TransformComponent>
		{
			// Attributes
			Vector3 m_position{};
			Quaternion m_rotation{ 1.0f, 0.0f, 0.0f, 0.0f };

			// Methods
			template <typename VisitorType>
			void accept(VisitorType& a_visitor)
			{
				a_visitor.visit("Position", m_position);
				a_visitor.visit("Rotation", m_rotation);
			}
		};
	}
}
