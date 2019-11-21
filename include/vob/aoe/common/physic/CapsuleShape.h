#pragma once

#include "ACollisionShape.h"
#include <bullet/BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>
#include <vob/aoe/core/visitor/Utils.h>
#include "vob/aoe/core/visitor/Aggregate.h"

namespace vob::aoe::common
{
	class CapsuleShape final
		: public vis::Aggregate<CapsuleShape, ACollisionShape>
	{
	public:
		// Methods
		btCollisionShape& getCollisionShape() override
		{
			return m_shape;
		}

		// Methods
		friend class vis::Aggregate<CapsuleShape, ACollisionShape>;
		template <typename VisitorType, typename ThisType>
		// ReSharper disable once CppMemberFunctionMayBeStatic
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
			auto t_radius = btScalar{ 0.5f };
			a_visitor.visit(vis::makeNameValuePair("Radius", t_radius));
			auto t_height = btScalar{ 1.0f };
			a_visitor.visit(vis::makeNameValuePair("Height", t_height));

			a_this.m_shape = btCapsuleShape(t_radius, t_height);
		}

	private:
		// Attributes
		btCapsuleShape m_shape{ 0.5f, 1.0f };
	};
}
