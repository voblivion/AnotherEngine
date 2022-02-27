#pragma once

#include "ACollisionShape.h"

#include <vob/aoe/common/space/Vector.h>
#include <vob/aoe/common/physic/Utils.h>

#include <vob/misc/visitor/name_value_pair.h>

#include <bullet/BulletCollision/CollisionShapes/btCylinderShape.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>


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

        template <typename VisitorType, typename Self>
        static bool accept(VisitorType& a_visitor, Self& a_this)
        {
            auto t_halfExtent = glm::vec3{ 0.5f, 0.5f, 0.5f };
            a_visitor.visit(misvi::nvp("HalfExtent", t_halfExtent));
            a_this.m_shape = btCylinderShape(common::toBtVector(t_halfExtent));
			return true;
        }

	private:
		// Attributes
		btCylinderShape m_shape{ btVector3{0.5f, 1.0f, 0.5f} };
	};
}

