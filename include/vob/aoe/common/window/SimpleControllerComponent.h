#pragma once

#include "vob/aoe/core/ecs/Component.h"
#include "vob/aoe/core/ecs/EntityId.h"
#include "vob/aoe/core/ecs/ComponentManager.h"
#include <vob/aoe/core/data/Handle.h>
#include <vob/aoe/common/time/Chrono.h>


namespace vob::aoe::common
{
	struct SimpleControllerComponent final
		: public ecs::AComponent
	{
		// Attributes
		glm::vec3 m_orientation{ 0.0f };
		data::Handle<ecs::ComponentManager> m_bullet;
		TimePoint m_lastBulletTime{};

		// Constructor
		explicit SimpleControllerComponent(data::ADatabase& a_database)
			: m_bullet{ a_database }
		{}
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
