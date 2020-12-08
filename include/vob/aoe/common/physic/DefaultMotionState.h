#pragma once
#include "AMotionState.h"
#include <LinearMath/btDefaultMotionState.h>


namespace vob::aoe::common
{
	class DefaultMotionState final
		: public AMotionState
	{
	public:
		// Methods
		btMotionState& getMotionState() override
		{
			return m_motionState;
		}

	private:
		// Attributes
		btDefaultMotionState m_motionState;
	};
}
