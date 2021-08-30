#pragma once

#include "ACollisionShape.h"
#include <bullet/BulletCollision/CollisionShapes/btSphereShape.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>
#include <vob/aoe/core/visitor/Utils.h>


namespace vob::aoe::common
{
	class SphereShape final
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
            auto t_radius = btScalar{ 1.0f };
            a_visitor.visit(vis::makeNameValuePair("Radius", t_radius));
            a_this.m_shape = btSphereShape(t_radius);
        }

	public: // TODO -> how to make accept friend ?
		// Attributes
		btSphereShape m_shape{ 1.0f };
	};
}

