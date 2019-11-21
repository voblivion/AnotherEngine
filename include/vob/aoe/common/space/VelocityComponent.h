#pragma once

#include <vob/aoe/api.h>
#include <vob/aoe/core/ecs/Component.h>
#include <glm/vec3.hpp>
#include <glm/ext/quaternion_float.hpp>

namespace vob::aoe::common
{
	struct VelocityComponent final
		: public vis::Aggregate<VelocityComponent, ecs::AComponent>
	{
		// Attributes
		glm::vec3 m_linear{};
		glm::quat m_angular{};

		// Methods
		friend class vis::Aggregate<VelocityComponent, ecs::AComponent>;
		template <typename VisitorType, typename ThisType>
		// ReSharper disable once CppMemberFunctionMayBeStatic
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{

		}
	};
}
