#pragma once
#include "AMotionState.h"
#include <LinearMath/btDefaultMotionState.h>
#include "vob/aoe/core/visitor/Aggregate.h"


namespace vob::aoe::common
{
	class DefaultMotionState final
		: public vis::Aggregate<DefaultMotionState, AMotionState>
	{
	public:
		// Methods
		btMotionState& getMotionState() override
		{
			return m_motionState;
		}

		// Methods
		friend class vis::Aggregate<DefaultMotionState, AMotionState>;
		template <typename VisitorType, typename ThisType>
		// ReSharper disable once CppMemberFunctionMayBeStatic
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{

		}

	private:
		// Attributes
		btDefaultMotionState m_motionState;
	};
}
