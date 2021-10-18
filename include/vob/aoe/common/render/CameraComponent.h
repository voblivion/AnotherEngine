#pragma once
#include <vob/aoe/core/ecs/Component.h>

namespace vob::aoe::common
{
	struct CameraComponent final
		: public ecs::AComponent
	{
		float fov{ 50.0f };
		float nearClip{ 0.1f };
		float farClip{ 1000.0f };
	};
}

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::CameraComponent, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(nvp("FOV", a_this.fov));
		a_visitor.visit(nvp("Near Clip", a_this.nearClip));
		a_visitor.visit(nvp("Far Clip", a_this.farClip));
	}
}
