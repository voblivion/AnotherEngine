#pragma once
#include <aoe/core/ecs/Component.h>


namespace aoe
{
	namespace common
	{
		struct CameraComponent final
			: public ecs::ComponentDefaultImpl<CameraComponent>
		{
			float fov{ 50.0f };
			float nearClip{ 0.1f };
			float farClip{ 1000.0f };

			// Methods
			template <typename VisitorType>
			// ReSharper disable once CppMemberFunctionMayBeStatic
			void accept(VisitorType& a_visitor)
			{

			}
		};
	}
}
