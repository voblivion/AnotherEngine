#pragma once

#include "ACollisionShape.h"
#include <bullet/BulletCollision/CollisionShapes/btCylinderShape.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>
#include <aoe/core/visitor/Utils.h>
#include "aoe/core/standard/Cloneable.h"
#include "aoe/core/visitor/Aggregate.h"
#include <aoe/common/space/Vector.h>
#include <aoe/common/physic/Utils.h>

namespace aoe
{
	namespace common
	{
		class CylinderShape final
			: public sta::CloneableDefaultImpl<ACollisionShape, CylinderShape
			, vis::Aggregate<CylinderShape, ACollisionShape>>
		{
		public:
			// Methods
			btCollisionShape& getCollisionShape() override
			{
				return m_shape;
			}

			// Methods
			friend class vis::Aggregate<CylinderShape, ACollisionShape>;
			template <typename VisitorType, typename ThisType>
			// ReSharper disable once CppMemberFunctionMayBeStatic
			static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
			{
				auto t_halfExtent = Vector3{ 0.5f, 0.5f, 0.5f };
				a_visitor.visit(vis::nvp("HalfExtent", t_halfExtent));
				a_this.m_shape = btCylinderShape(toBtVector(t_halfExtent));
			}

		private:
			// Attributes
			btCylinderShape m_shape{ btVector3{0.5f, 1.0f, 0.5f} };
		};
	}
}
