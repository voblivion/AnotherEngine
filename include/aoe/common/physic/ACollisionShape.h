#pragma once

#include "aoe/core/standard/ADynamicType.h"
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>

namespace aoe
{
	namespace common
	{
		class ACollisionShape
			: public sta::ADynamicType
			, public sta::ICloneable<ACollisionShape>
		{
		public:
			virtual btCollisionShape& getCollisionShape() = 0;
		};
	}
}
