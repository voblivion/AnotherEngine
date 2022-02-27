#pragma once


namespace vob::aoe::common
{
	struct CameraComponent final
	{
		float fov{ 70.0f };
		float nearClip{ 0.1f };
		float farClip{ 1000.0f };
	};
}

namespace vob::misvi
{
	template <typename VisitorType, typename ThisType>
	requires std::is_same_v<std::remove_cvref_t<ThisType>, aoe::common::CameraComponent>
	bool accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(nvp("FOV", a_this.fov));
		a_visitor.visit(nvp("Near Clip", a_this.nearClip));
		a_visitor.visit(nvp("Far Clip", a_this.farClip));
		return true;
	}
}
