#pragma once

#include <vob/aoe/common/time/Chrono.h>
#include <vob/aoe/core/visitor/Aggregate.h>
#include <vob/aoe/core/ecs/Component.h>

namespace vob::aoe::common
{
	struct LifetimeComponent final
		: public vis::Aggregate<LifetimeComponent, ecs::AComponent>
	{
		// Attributes
		float m_remainingTime{ 1.0f };

		// Methods
		friend class vis::Aggregate<LifetimeComponent, ecs::AComponent>;
		template <typename VisitorType, typename ThisType>
		// ReSharper disable once CppMemberFunctionMayBeStatic
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
			a_visitor.visit(vis::nvp("RemainingTime", a_this.m_remainingTime));
		}
	};
}
