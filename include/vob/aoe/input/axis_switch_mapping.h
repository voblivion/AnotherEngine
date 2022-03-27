#pragma once

#include <vob/aoe/input/basic_axis_mapping.h>
#include <vob/aoe/input/basic_switch_mapping.h>
#include <vob/aoe/input/physical_inputs.h>

#include <vob/misc/std/polymorphic_ptr.h>

#include <cassert>


namespace vob::aoein
{
	class axis_switch_mapping
		: public basic_switch_mapping
	{
	public:
		// CREATORS
		axis_switch_mapping(
			mistd::polymorphic_ptr<basic_axis_mapping> a_axisMapping,
			float a_pressThreshold,
			float a_releaseThreshold,
			bool a_isUp)
			: m_axisMapping{ std::move(a_axisMapping) }
			, m_pressThreshold{ a_pressThreshold }
			, m_releaseThreshold{ a_releaseThreshold }
			, m_isUp{ a_isUp }
		{
			assert(m_axisMapping != nullptr);
		}

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
		void update(physical_inputs const& a_physicalInput, misph::measure_time a_elapsedTime) override
		{
			m_axisMapping->update(a_physicalInput, a_elapsedTime);

			auto const newValue = m_axisMapping->get_value();
			if (!m_isPressed
				&& ((m_isUp && newValue >= m_pressThreshold) || (!m_isUp && m_pressThreshold >= newValue)))
			{
				m_isPressed = true;
				m_changed = true;
			}
			else if (m_isPressed
				&& ((m_isUp && newValue <= m_releaseThreshold) || (!m_isUp && m_releaseThreshold >= newValue)))
			{
				m_isPressed = false;
				m_changed = true;
			}
			m_value = newValue;
		}

		// DATA
		mistd::polymorphic_ptr<basic_axis_mapping> m_axisMapping;
		float m_pressThreshold;
		float m_releaseThreshold;
		bool m_isUp;
	private:
		bool m_isPressed = false;
		bool m_changed = false;
		float m_value = -1.0f;
	};
}
