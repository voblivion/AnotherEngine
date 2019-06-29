#pragma once
#include "aoe/core/ecs/Component.h"
#include "aoe/core/ecs/EntityId.h"


namespace aoe
{
	namespace common
	{
		struct SimpleControllerComponent final
			: public ecs::ComponentDefaultImpl<SimpleControllerComponent>
		{
			glm::vec3 m_orientation{ 0.0f };

			// Methods
			template <typename VisitorType>
			// ReSharper disable once CppMemberFunctionMayBeStatic
			void accept(VisitorType& a_visitor)
			{

			}
		};
	}
}
