#pragma once
#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/core/visitor/Aggregate.h>

namespace vob::aoe::common
{
	struct CameraComponent final
		: public vis::Aggregate<CameraComponent, ecs::AComponent>
	{
		float fov{ 50.0f };
		float nearClip{ 0.1f };
		float farClip{ 1000.0f };

		// Methods
		friend class vis::Aggregate<CameraComponent, ecs::AComponent>;
		template <typename VisitorType, typename ThisType>
		// ReSharper disable once CppMemberFunctionMayBeStatic
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{

		}
	};
}
