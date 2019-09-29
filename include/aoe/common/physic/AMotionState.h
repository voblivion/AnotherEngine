#pragma once
#include "aoe/core/standard/ADynamicType.h"
#include "aoe/core/standard/TypeRegistry.h"
#include <LinearMath/btMotionState.h>


namespace aoe
{
	namespace common
	{
		class AMotionState
			: public sta::ADynamicType
			, public sta::ICloneable<AMotionState>
		{
		public:
			virtual btMotionState& getMotionState() = 0;
		};
	}
}
