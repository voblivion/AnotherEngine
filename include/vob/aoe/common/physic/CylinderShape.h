#pragma once

#include "ACollisionShape.h"
#include <bullet/BulletCollision/CollisionShapes/btCylinderShape.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>
#include <vob/aoe/core/visitor/Utils.h>
#include <vob/aoe/common/space/Vector.h>
#include <vob/aoe/common/physic/Utils.h>

namespace vob::aoe::common
{
	class CylinderShape final
		: public ACollisionShape
	{
	public:
		// Methods
		btCollisionShape& getCollisionShape() override
		{
			return m_shape;
		}

	public: // TODO -> how to make accept friend ?
		// Attributes
		btCylinderShape m_shape{ btVector3{0.5f, 1.0f, 0.5f} };
	};
}

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::CylinderShape, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
		auto t_halfExtent = vec3{ 0.5f, 0.5f, 0.5f };
		a_visitor.visit(vis::nvp("HalfExtent", t_halfExtent));
		a_this.m_shape = btCylinderShape(common::toBtVector(t_halfExtent));
	}
}
