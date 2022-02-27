#pragma once

#include <vob/aoe/api.h>

#include <glm/vec3.hpp>
#include <glm/ext/quaternion_float.hpp>


namespace vob::aoe::common
{
	struct VelocityComponent final
	{
		// Attributes
		glm::vec3 m_linear{};
		glm::quat m_angular{};
	};
}

namespace vob::misvi
{
	template <typename VisitorType, typename ThisType>
	requires std::is_same_v<std::remove_cvref_t<ThisType>, vob::aoe::common::VelocityComponent>
	bool accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(misvi::nvp("Linear Velocity", a_this.m_linear));
		a_visitor.visit(misvi::nvp("Angular Velocity", a_this.m_angular));
		return true;
	}
}
