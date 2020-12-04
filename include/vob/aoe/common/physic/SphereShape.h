#pragma once

#include "ACollisionShape.h"
#include <bullet/BulletCollision/CollisionShapes/btSphereShape.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>
#include <vob/aoe/core/visitor/Utils.h>
#include "vob/aoe/core/visitor/Aggregate.h"

namespace vob::aoe::common
{
	class SphereShape final
		: public vis::Aggregate<SphereShape, ACollisionShape>
	{
	public:
		// Methods
		btCollisionShape& getCollisionShape() override
		{
			return m_shape;
		}

		// Methods
		friend class vis::Aggregate<SphereShape, ACollisionShape>;
		template <typename VisitorType, typename ThisType>
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
			auto t_radius = btScalar{ 1.0f };
			a_visitor.visit(vis::makeNameValuePair("Radius", t_radius));
			a_this.m_shape = btSphereShape(t_radius);
		}

	private:
		// Attributes
		btSphereShape m_shape{ 1.0f };
	};
}
