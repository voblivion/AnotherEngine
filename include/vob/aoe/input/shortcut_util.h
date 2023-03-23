#pragma once

#include <vob/aoe/input/keyboard.h>
#include <vob/aoe/input/mouse.h>
#include <vob/aoe/input/physical_switch_mapping.h>
#include <vob/aoe/input/physical_switch_reference.h>
#include <vob/aoe/input/double_switch_axis_mapping.h>

#include <vob/misc/std/polymorphic_ptr_util.h>


namespace vob::aoein::shortcut_util
{
	auto make_switch(physical_switch_reference a_physicalSwitch)
	{
		return mistd::polymorphic_ptr_util::make<physical_switch_mapping>(a_physicalSwitch);
	}

	auto make_axis(physical_switch_reference a_physicalSwitchUp, physical_switch_reference a_physicalSwitchDown)
	{
		return mistd::polymorphic_ptr_util::make<double_switch_axis_mapping>(
			make_switch(a_physicalSwitchUp),
			make_switch(a_physicalSwitchDown),
			0.0f,
			1000.0f,
			1000.0f);
	}
}
