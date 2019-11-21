#pragma once

#include "vob/aoe/core/type/ADynamicType.h"
#include <bullet/BulletDynamics/Dynamics/btDynamicsWorld.h>

namespace vob::aoe::common
{
	class ADynamicsWorldHolder
		: public type::ADynamicType
	{
	public:
		virtual btDynamicsWorld& getDynamicsWorld() = 0;
	};
}
