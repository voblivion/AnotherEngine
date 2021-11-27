#pragma once

#include <vob/aoe/api.h>
#include <vob/aoe/ecs/Component.h>
#include <glm/vec3.hpp>
#include <glm/ext/quaternion_float.hpp>


namespace vob::aoe::common
{
	struct VelocityComponent final
		: public aoecs::AComponent
	{
		// Attributes
		glm::vec3 m_linear{};
		glm::quat m_angular{};
	};
}

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::VelocityComponent, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(vis::makeNameValuePair("Linear Velocity", a_this.m_linear));
		a_visitor.visit(vis::makeNameValuePair("Angular Velocity", a_this.m_angular));
	}
}
