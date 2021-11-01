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

		template <typename VisitorType>
		void accept(VisitorType& a_visitor)
		{
			auto t_radius = btScalar{ 0.5f };
			a_visitor.visit(vis::makeNameValuePair("Radius", t_radius));
			auto t_height = btScalar{ 1.0f };
			a_visitor.visit(vis::makeNameValuePair("Height", t_height));

			m_shape = btCapsuleShape(t_radius, t_height);
		}

		template <typename VisitorType>
		void accept(VisitorType& a_visitor) const
		{
			static_assert(false && "TODO");
		}

	private:
		// Attributes
		btCapsuleShape m_shape{ 0.5f, 1.0f };
	};
}
