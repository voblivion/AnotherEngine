#pragma once

#include <vob/aoe/common/time/Chrono.h>

#include "vob/aoe/ecs/entity_id.h"
#include "vob/aoe/ecs/component_manager.h"

#include <vob/misc/visitor/name_value_pair.h>



namespace vob::aoe::common
{
	struct SimpleControllerComponent final
	{
		// Attributes
		glm::vec3 m_orientation{ 0.0f };
		glm::vec3 m_headOrientation{ 0.0f };
		std::shared_ptr<aoecs::component_manager const> m_bullet;
		TimePoint m_lastBulletTime{};
		float m_fallVelocity = 0.0f;
		TimePoint m_lastJumpTime{};
	};
}

namespace vob::misvi
{
	template <typename VisitorType, typename ThisType>
	requires std::is_same_v<std::remove_cvref_t<ThisType>, aoe::common::SimpleControllerComponent>
	bool accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(misvi::nvp("BulletArchetype", a_this.m_bullet));
		return true;
	}
}
