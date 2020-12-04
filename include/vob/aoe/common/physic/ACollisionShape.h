#pragma once

#include <vob/aoe/core/type/ADynamicType.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>

namespace vob::aoe::common
{
	class ACollisionShape
		: public type::ADynamicType
	{
	public:
		virtual btCollisionShape& getCollisionShape() = 0;
	};
}
