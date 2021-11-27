#pragma once
#include "vob/aoe/core/type/ADynamicType.h"
#include <bullet/LinearMath/btMotionState.h>


namespace vob::aoe::common
{
	class AMotionState
		: public type::ADynamicType
	{
	public:
		virtual btMotionState& getMotionState() = 0;
	};
}
