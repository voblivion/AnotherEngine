#pragma once

#include <vob/aoe/input/basic_axis_mapping.h>
#include <vob/aoe/input/basic_switch_mapping.h>
#include <vob/aoe/input/physical_switch_reference.h>

#include <vob/misc/physics/measure_literals.h>
#include <vob/misc/std/polymorphic_ptr.h>

#include <cassert>


namespace vob::aoein
{
	using namespace misph;

	class double_switch_axis_mapping
		: public basic_axis_mapping
	{
	public:
		// CREATORS
		double_switch_axis_mapping(
			mistd::polymorphic_ptr<basic_switch_mapping> a_upSwitchMapping,
			mistd::polymorphic_ptr<basic_switch_mapping> a_downSwitchMapping,
			float a_restValue,
			float a_upStrength,
			float a_downStrength)
			: m_upSwitchMapping{ std::move(a_upSwitchMapping) }
			, m_downSwitchMapping{ std::move(a_downSwitchMapping) }
			, m_restValue{ a_restValue }
			, m_upStrength{ a_upStrength }
			, m_downStrength{ a_downStrength }
		{
			assert(m_upSwitchMapping != nullptr);
			assert(m_downSwitchMapping != nullptr);
		}

		// ACCESSORS
		float get_value() const override
		{
			return m_value;
		}

		// MANIPULATORS
		void update(physical_inputs const& a_physicalInput, misph::measure_time a_elapsedTime) override
		{
			m_upSwitchMapping->update(a_physicalInput, a_elapsedTime);
			m_downSwitchMapping->update(a_physicalInput, a_elapsedTime);

			if (m_upSwitchMapping->is_pressed())
			{
				m_value = std::min(1.0f, m_value + a_elapsedTime.get_value() * m_upStrength);
			}
			else if (m_downSwitchMapping->is_pressed())
			{
				m_value = std::max(-1.0f, m_value - a_elapsedTime.get_value() * m_downStrength);
			}
			else if (m_value < m_restValue)
			{
				m_value = std::min(m_restValue, m_value + a_elapsedTime.get_value() * m_upStrength);
			}
			else
			{
				m_value = std::max(m_restValue, m_value - a_elapsedTime.get_value() * m_downStrength);
			}
		}

		// DATA
		mistd::polymorphic_ptr<basic_switch_mapping> m_upSwitchMapping;
		mistd::polymorphic_ptr<basic_switch_mapping> m_downSwitchMapping;
		float m_restValue = 0.0f;
		float m_upStrength = 0.5f;
		float m_downStrength = 0.5f;
	private:
		float m_value = -1.0f;
	};
}
