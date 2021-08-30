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

        template <typename VisitorType, typename Self>
        static void accept(VisitorType& a_visitor, Self& a_this)
        {
            auto t_halfExtent = vec3{ 0.5f, 0.5f, 0.5f };
            a_visitor.visit(vis::makeNameValuePair("HalfExtent", t_halfExtent));
            a_this.m_shape = btBoxShape(common::toBtVector(t_halfExtent));
        }

	private:
		// Attributes
		btBoxShape m_shape{btVector3{0.5f, 0.5f, 0.5f}};
	};
}

