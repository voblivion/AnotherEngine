#pragma once

#include <vob/misc/visitor/name_value_pair.h>


namespace vob::aoe::common
{
	struct TestComponent
	{
		float t{ 0.0f };
		int type{ 0 };
	};
}

namespace vob::misvi
{
	template <typename VisitorType, typename ThisType>
	requires std::is_same_v<std::remove_cvref_t<ThisType>, aoe::common::TestComponent>
	bool accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(misvi::nvp("Type", a_this.type));
		return true;
	}
}
