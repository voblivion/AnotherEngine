#pragma once

#include <vob/aoe/input/basic_axis_mapping.h>
#include <vob/aoe/input/mouse_move_axis_reference.h>
#include <vob/aoe/input/mouse.h>


namespace vob::aoein
{
	class mouse_move_axis_mapping
		: public basic_axis_mapping
	{
	public:
		// CREATORS
		mouse_move_axis_mapping(mouse_move_axis_reference a_mouseMoveAxisReference, float a_sensitivity)
			: m_mouseMoveAxisReference{ a_mouseMoveAxisReference }
			, m_sensitivity{ a_sensitivity }
		{}

		// ACCESSORS
		float get_value() const override
		{
			return m_value;
		}

		// MANIPULATORS
		void update(physical_inputs const& a_physicalInput, misph::measure_time a_elapsedTime) override
		{
			m_value = m_mouseMoveAxisReference.get_value(a_physicalInput) * m_sensitivity
				* a_elapsedTime.get_value();
		}

		// DATA
		mouse_move_axis_reference m_mouseMoveAxisReference;
		float m_sensitivity;
	private:
		float m_value = 0.0f;
	};
}
