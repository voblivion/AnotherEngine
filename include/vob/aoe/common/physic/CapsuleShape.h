#pragma once

#include "ACollisionShape.h"
#include <bullet/BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>
#include <vob/aoe/core/visitor/Utils.h>

namespace vob::aoe::common
{
	class CapsuleShape final
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
		btCapsuleShape m_shape{ 0.5f, 1.0f };
	};
}

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::CapsuleShape, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
		auto t_radius = btScalar{ 0.5f };
		a_visitor.visit(vis::makeNameValuePair("Radius", t_radius));
		auto t_height = btScalar{ 1.0f };
		a_visitor.visit(vis::makeNameValuePair("Height", t_height));

		a_this.m_shape = btCapsuleShape(t_radius, t_height);
	}
}
