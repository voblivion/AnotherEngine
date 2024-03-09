#pragma once

#include <vob/misc/visitor/macros.h>

#include <vob/misc/std/message_macros.h>

#include <numbers>


namespace vob::aoegl
{
	struct camera_component final
	{
		float m_fov = 70.0f / 180.0f * std::numbers::pi_v<float>;
		float m_nearClip{ 0.1f };
		float m_farClip{ 1000.0f };
	};
}

namespace vob::misvi
{
	VOB_MISVI_ACCEPT(vob::aoegl::camera_component)
	{
		VOB_MISVI_NVP("Field Of View", fov);
		VOB_MISVI_NVP("Near Clip", nearClip);
		VOB_MISVI_NVP("Far Clip", farClip);
		return true;
	}
}
