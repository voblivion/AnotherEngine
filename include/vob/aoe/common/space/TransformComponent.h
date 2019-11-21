#pragma once

#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/core/visitor/Aggregate.h>
#include <vob/aoe/common/space/Vector.h>
#include <vob/aoe/common/space/Quaternion.h>

namespace vob::aoe::common
{
	const Vector3 right = Vector3{ 1.0f, 0.0f, 0.0f };
	const Vector3 front = Vector3{ 0.0f, 1.0f, 0.0f };
	const Vector3 up = Vector3{ 0.0f, 0.0f, 1.0f };

	struct TransformComponent final
		: public vis::Aggregate<TransformComponent, ecs::AComponent>
	{
		// Attributes
		Vector3 m_position{};
		Quaternion m_rotation{ 1.0f, 0.0f, 0.0f, 0.0f };

	private:
		friend class vis::Aggregate<TransformComponent, ecs::AComponent>;
		template <typename VisitorType, typename ThisType>
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
			a_visitor.visit(vis::makeNameValuePair("Position", a_this.m_position));
			a_visitor.visit(vis::makeNameValuePair("Rotation", a_this.m_rotation));
		}
	};
}
