#pragma once

#include "ACollisionShape.h"
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>
#include <vob/aoe/core/visitor/Utils.h>
#include <vob/aoe/core/visitor/Traits.h>
#include <vob/aoe/common/space/Vector.h>
#include <vob/aoe/common/physic/Utils.h>

namespace vob::aoe::common
{
	class BoxShape final
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
		btBoxShape m_shape{btVector3{0.5f, 0.5f, 0.5f}};
	};
}

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::BoxShape, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
		auto t_halfExtent = vec3{ 0.5f, 0.5f, 0.5f };
		a_visitor.visit(vis::makeNameValuePair("HalfExtent", t_halfExtent));
		a_this.m_shape = btBoxShape(common::toBtVector(t_halfExtent));
	}
}
