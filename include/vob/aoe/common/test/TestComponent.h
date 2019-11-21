#pragma once
#include "vob/aoe/core/ecs/Component.h"


namespace vob::aoe::common
{
	struct TestComponent
		: public vis::Aggregate<TestComponent, ecs::AComponent>
	{
		float t{ 0.0f };
		int type{ 0 };


	private:
		friend class vis::Aggregate<TestComponent, ecs::AComponent>;
		template <typename VisitorType, typename ThisType>
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
			a_visitor.visit(vis::makeNameValuePair("Type", a_this.type));
		}
	};
}
