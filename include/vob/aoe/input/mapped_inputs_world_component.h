#pragma once

#include <vob/aoe/input/basic_axis_mapping.h>
#include <vob/aoe/input/basic_switch_mapping.h>

#include <vob/misc/std/polymorphic_ptr.h>

#include <vector>


namespace vob::aoein
{
	struct mapped_inputs_world_component
	{
		std::pmr::vector<mistd::polymorphic_ptr<basic_axis_mapping>> m_axes;
		std::pmr::vector<mistd::polymorphic_ptr<basic_switch_mapping>> m_switches;
	};
}
