#pragma once

#include <vob/misc/visitor/macros.h>


namespace vob::aoegl
{
	struct camera_component final
	{
#pragma message("TODO fov stored in degree but loaded in radians?")
		float m_fovDegree{ 70.0f };
		float m_nearClip{ 0.1f };
		float m_farClip{ 1000.0f };
	};
}

namespace vob::misvi
{
	VOB_MISVI_ACCEPT(vob::aoegl::camera_component)
	{
		VOB_MISVI_NVP("FOV", fovDegree);
		VOB_MISVI_NVP("Near Clip", nearClip);
		VOB_MISVI_NVP("Far Clip", farClip);
		return true;
	}
}
