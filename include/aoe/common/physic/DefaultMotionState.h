#pragma once
#include "AMotionState.h"
#include <LinearMath/btDefaultMotionState.h>
#include "aoe/core/standard/Cloneable.h"
#include "aoe/core/visitor/Aggregate.h"


namespace aoe
{
	namespace common
	{
		class DefaultMotionState final
			: public sta::CloneableDefaultImpl<AMotionState, DefaultMotionState
			, vis::Aggregate<DefaultMotionState, AMotionState>>
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
}
