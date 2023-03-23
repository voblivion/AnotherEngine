#pragma once

#include <glm/glm.hpp>

#include <vector>


namespace vob::aoegl
{
	struct rig_bind_pose_component
	{
#pragma message("TODO resourcce?")
		std::pmr::vector<glm::mat4> m_jointRelativeBindTransforms;
		std::pmr::vector<glm::mat4> m_jointInverseBindTransforms;
	};
}

namespace vob::misvi
{
	VOB_MISVI_ACCEPT(aoegl::rig_bind_pose_component)
	{
		return true;
	}
}
