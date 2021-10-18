#pragma once

#include "vob/aoe/core/ecs/Component.h"
#include "vob/aoe/core/ecs/EntityId.h"
#include "vob/aoe/core/ecs/ComponentManager.h"
#include <vob/aoe/common/time/Chrono.h>


namespace vob::aoe::common
{
	struct SimpleControllerComponent final
		: public ecs::AComponent
	{
		// Attributes
		glm::vec3 m_orientation{ 0.0f };
		std::shared_ptr<ecs::ComponentManager const> m_bullet;
		TimePoint m_lastBulletTime{};
		float m_fallVelocity = 0.0f;
	};
}

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::SimpleControllerComponent, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(vis::nvp("BulletArchetype", a_this.m_bullet));
	}
}
