#pragma once

#include <vob/aoe/input/basic_axis_mapping.h>
#include <vob/aoe/input/physical_axis_reference.h>
#include <vob/aoe/input/physical_inputs.h>


namespace vob::aoein
{
	class physical_axis_mapping
		: public basic_axis_mapping
	{
	public:
		// CREATORS
		physical_axis_mapping(physical_axis_reference a_physicalAxisReference, float a_deadZone)
			: m_physicalAxisReference(a_physicalAxisReference)
			, m_deadZone{ a_deadZone }
		{}

		// ACCESSORS
		float get_value() const override
		{
			return m_value;
		}

		// MANIPULATORS
		void update(physical_inputs const& a_physicalInput, misph::measure_time) override
		{
			m_value = m_physicalAxisReference.get_value(a_physicalInput);
			if (std::abs(m_value) < m_deadZone)
			{
				m_value = 0.0f;
			}
		}

		// DATA
		physical_axis_reference m_physicalAxisReference;
		float m_deadZone;
	private:
		float m_value = 0.0f;
	};
}
