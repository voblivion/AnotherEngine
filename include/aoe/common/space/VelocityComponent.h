#pragma once

#include <aoe/common/Export.h>
#include <aoe/core/ecs/Component.h>
#include <glm/vec3.hpp>
#include <glm/ext/quaternion_float.hpp>

namespace aoe
{
	namespace common
	{
		struct AOE_COMMON_API VelocityComponent final
			: public ecs::ComponentDefaultImpl<VelocityComponent>
		{
			// Attributes
			glm::vec3 m_linear{};
			glm::quat m_angular{};

			// Methods
			template <typename VisitorType>
			// ReSharper disable once CppMemberFunctionMayBeStatic
			void accept(VisitorType& a_visitor)
			{

			}
		};
	}
}
