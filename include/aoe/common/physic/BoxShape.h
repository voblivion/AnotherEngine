#pragma once

#include "ACollisionShape.h"
#include <BulletCollision/CollisionShapes/btBoxShape.h>
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
		class BoxShape final
			: public sta::CloneableDefaultImpl<ACollisionShape, BoxShape
			, vis::Aggregate<BoxShape, ACollisionShape>>
		{
		public:
			// Methods
			btCollisionShape& getCollisionShape() override
			{
				return m_shape;
			}

			// Methods
			friend class vis::Aggregate<BoxShape, ACollisionShape>;
			template <typename VisitorType, typename ThisType>
			// ReSharper disable once CppMemberFunctionMayBeStatic
			static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
			{
				auto t_halfExtent = Vector3{ 0.5f, 0.5f, 0.5f };
				a_visitor.visit(vis::makeNameValuePair("HalfExtent", t_halfExtent));
				a_this.m_shape = btBoxShape(toBtVector(t_halfExtent));
			}

		private:
			// Attributes
			btBoxShape m_shape{btVector3{0.5f, 0.5f, 0.5f}};
		};
	}
}
