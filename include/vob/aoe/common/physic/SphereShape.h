#pragma once

#include "ACollisionShape.h"
#include <bullet/BulletCollision/CollisionShapes/btSphereShape.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>

#include <vob/misc/visitor/name_value_pair.h>



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
        static bool accept(VisitorType& a_visitor, Self& a_this)
        {
            auto t_radius = btScalar{ 1.0f };
            a_visitor.visit(misvi::nvp("Radius", t_radius));
            a_this.m_shape = btSphereShape(t_radius);
			return true;
        }

	public: // TODO -> how to make accept friend ?
		// Attributes
		btSphereShape m_shape{ 1.0f };
	};
}

