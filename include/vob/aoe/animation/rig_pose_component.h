#pragma once

#include <vob/aoe/rendering/rig.h>


namespace vob::aoegl
{
	struct rig_pose_component
	{
		rig m_rig = create_default_rig();
	};
}

namespace vob::misvi
{
	VOB_MISVI_ACCEPT(aoegl::rig_pose_component)
	{
		return true;
	}
}
