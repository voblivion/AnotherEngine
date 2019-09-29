#pragma once

#include "aoe/core/standard/ADynamicType.h"
#include <bullet/BulletDynamics/Dynamics/btDynamicsWorld.h>
#include "aoe/core/standard/Cloneable.h"

namespace aoe
{
	namespace common
	{
		class ADynamicsWorldHolder
			: public sta::ADynamicType
			, public sta::ICloneable<ADynamicsWorldHolder>
		{
		public:
			virtual btDynamicsWorld& getDynamicsWorld() = 0;
		};
	}
}
