#pragma once

#include <vob/aoe/input/basic_axis_mapping.h>
#include <vob/aoe/input/basic_switch_mapping.h>

#include <vob/misc/std/polymorphic_ptr.h>

#include <vector>


namespace vob::aoein
{
	class mapped_inputs_world_component
	{
	public:
		mapped_inputs_world_component() {}
		mapped_inputs_world_component(mapped_inputs_world_component&&) = default;

		std::pmr::vector<mistd::polymorphic_ptr<basic_axis_mapping>> m_axes;
		std::pmr::vector<mistd::polymorphic_ptr<basic_switch_mapping>> m_switches;

		mapped_inputs_world_component& operator=(mapped_inputs_world_component&&) = default;
	};
}
