#pragma once

#include <vob/aoe/input/basic_switch_mapping.h>
#include <vob/aoe/input/physical_switch_reference.h>
#include <vob/aoe/input/physical_inputs.h>


namespace vob::aoein
{
	class physical_switch_mapping
		: public basic_switch_mapping
	{
	public:
		// CREATORS
		physical_switch_mapping(physical_switch_reference a_physicalSwitchReference)
			: m_physicalSwitchReference(a_physicalSwitchReference)
		{}

		// ACCESSORS
		bool is_pressed() const override
		{
			return m_isPressed;
		}

		bool changed() const override
		{
			return m_changed;
		}
		
		// MANIPULATORS
		void update(physical_inputs const& a_physicalInput, misph::measure_time) override
		{
			m_isPressed = m_physicalSwitchReference.is_pressed(a_physicalInput);
			m_changed = m_physicalSwitchReference.changed(a_physicalInput);
		}

		// DATA
		physical_switch_reference m_physicalSwitchReference;
	private:
		bool m_isPressed = false;
		bool m_changed = false;
	};
}
