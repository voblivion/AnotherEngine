#pragma once

#include <vob/aoe/core/ecs/Component.h>
#include "vob/aoe/core/visitor/Aggregate.h"

namespace vob::aoe::common
{
	struct UIComponent final
		: public vis::Aggregate<UIComponent, ecs::AComponent>
	{
		// Attributes

		// Constructor
		explicit UIComponent()
		{}

		// Methods
		friend class vis::Aggregate<UIComponent, ecs::AComponent>;
		template <typename VisitorType, typename ThisType>
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
		}
	};
}
