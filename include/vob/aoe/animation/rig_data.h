#pragma once

#include <vob/aoe/rendering/primitives.h>

#include <glm/glm.hpp>

#include <optional>
#include <string>
#include <vector>


namespace vob::aoegl
{

	struct joint_data
	{
		std::optional<std::size_t> m_parentIndex;
		std::pmr::string m_name;
		glm::mat4 m_relativeTransform;
	};

	struct rig_data
	{
		std::pmr::vector<joint_data> m_joints;
	};
}
