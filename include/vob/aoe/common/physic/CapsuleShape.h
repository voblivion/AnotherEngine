#pragma once

#include "ACollisionShape.h"

#include <vob/misc/visitor/name_value_pair.h>

#include <bullet/BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>


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

		template <typename VisitorType>
		bool accept(VisitorType& a_visitor)
		{
			auto t_radius = btScalar{ 0.5f };
			a_visitor.visit(misvi::nvp("Radius", t_radius));
			auto t_height = btScalar{ 1.0f };
			a_visitor.visit(misvi::nvp("Height", t_height));

			m_shape = btCapsuleShape(t_radius, t_height);
			return true;
		}

		template <typename VisitorType>
		bool accept(VisitorType& a_visitor) const
		{
			static_assert(false && "TODO");
			return true;
		}

	private:
		// Attributes
		btCapsuleShape m_shape{ 0.5f, 1.0f };
	};
}
