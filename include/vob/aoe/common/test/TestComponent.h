#pragma once

#include "vob/aoe/ecs/Component.h"


namespace vob::aoe::common
{
	struct TestComponent
		: public aoecs::AComponent
	{
		float t{ 0.0f };
		int type{ 0 };
	};
}

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::TestComponent, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(vis::makeNameValuePair("Type", a_this.type));
	}
}
