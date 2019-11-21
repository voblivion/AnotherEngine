#pragma once

#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/core/visitor/Aggregate.h>

#include <vob/aoe/common/space/Vector.h>


namespace vob::aoe::common::gui
{
	struct CanvasComponent final
		: public vis::Aggregate<CanvasComponent, ecs::AComponent>
	{
		// Attributes
		Vector2 m_size{};

		// Methods
		friend class vis::Aggregate<CanvasComponent, ecs::AComponent>;
		template <typename VisitorType, typename ThisType>
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{

		}
	};
}
