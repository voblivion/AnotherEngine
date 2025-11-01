#pragma once

#include <vob/aoe/input/InputBindings.h>
#include <vob/aoe/input/InputReference.h>


namespace vob::aoein
{
	class DefaultSwitchInputBinding : public ISwitchInputBinding
	{
	public:
		explicit DefaultSwitchInputBinding(SwitchReference const a_switchReference)
			: m_switchReference{ a_switchReference }
		{
		}

		bool hasChanged() const override
		{
			return m_hasChanged;
		}

		bool isPressed() const override
		{
			return m_isPressed;
		}

		void update(Inputs const& a_inputs, misph::measure_time const a_elapsedTime) override
		{
			m_hasChanged = m_switchReference.hasChanged(a_inputs);
			m_isPressed = m_switchReference.isPressed(a_inputs);
		}

	private:
		SwitchReference m_switchReference;
		bool m_hasChanged = false;
		bool m_isPressed = false;
	};

	class AxisSwitchInputBinding : public ISwitchInputBinding
	{
	public:
		explicit AxisSwitchInputBinding(
			std::shared_ptr<IAxisInputBinding> a_axisBinding,
			float const a_pressThreshold = 0.0f,
			float const a_releaseThreshold = 0.0f,
			bool a_isUp = true
		)
			: m_axisBinding{ std::move(a_axisBinding) }
			, m_pressThreshold{ a_pressThreshold }
			, m_releaseThreshold{ a_releaseThreshold }
			, m_isUp{ a_isUp }
		{
		}

		bool hasChanged() const override
		{
			return m_hasChanged;
		}

		bool isPressed() const override
		{
			return m_isPressed;
		}

		void update(Inputs const& a_inputs, misph::measure_time const a_elapsedTime) override
		{
			m_axisBinding->update(a_inputs, a_elapsedTime);

			auto const newValue = m_axisBinding->getValue();
			if (!m_isPressed
				&& ((m_isUp && newValue >= m_pressThreshold)
					|| (!m_isUp && m_pressThreshold >= newValue)))
			{
				m_isPressed = true;
				m_hasChanged = true;
			}
			else if (m_isPressed
				&& ((m_isUp && newValue <= m_releaseThreshold)
					|| (!m_isUp && m_releaseThreshold <= newValue)))
			{
				m_isPressed = false;
				m_hasChanged = true;
			}
		}

	private:
		std::shared_ptr<IAxisInputBinding> m_axisBinding;
		float m_pressThreshold;
		float m_releaseThreshold;
		bool m_isUp;
		bool m_isPressed = false;
		bool m_hasChanged = false;
	};

	class DefaultAxisInputBinding : public IAxisInputBinding
	{
	public:
		explicit DefaultAxisInputBinding(
			AxisReference const a_axisReference,
			float const a_sensitivity = 1.0f,
			float const a_deadZonePosition = 0.0f,
			float const a_deadZoneRadius = 0.0f
		)
			: m_axisReference{ a_axisReference }
			, m_sensitivity{ a_sensitivity }
			, m_deadZonePosition{ a_deadZonePosition }
			, m_deadZoneRadius{ a_deadZoneRadius }
		{
		}

		float getChange() const override
		{
			return m_change;
		}

		float getValue() const override
		{
			return m_value;
		}

		void update(Inputs const& a_inputs, misph::measure_time const a_elapsedTime) override
		{
			auto const oldValue = m_value;
			m_value = m_axisReference.getValue(a_inputs) * m_sensitivity;
			if (std::abs(m_value - m_deadZonePosition) < m_deadZoneRadius)
			{
				m_value = m_deadZonePosition;
			}
			m_change = m_value - oldValue;
		}

	private:
		AxisReference m_axisReference;
		float m_sensitivity = 1.0f;
		float m_deadZonePosition = 0.0f;
		float m_deadZoneRadius = 0.0f;
		float m_change = 0.0f;
		float m_value = -1.0f;
	};

	class DerivedAxisInputBinding : public IAxisInputBinding
	{
	public:
		explicit DerivedAxisInputBinding(std::shared_ptr<IAxisInputBinding> a_baseAxisBinding)
			: m_baseAxisBinding{ std::move(a_baseAxisBinding) }
		{
		}

		float getChange() const override
		{
			return m_change;
		}

		float getValue() const override
		{
			return m_value;
		}

		void update(Inputs const& a_inputs, misph::measure_time const a_elapsedTime) override
		{
			m_baseAxisBinding->update(a_inputs, a_elapsedTime);

			auto const oldValue = m_value;
			m_value = m_baseAxisBinding->getChange() / a_elapsedTime.get_value();
			m_change = m_value - oldValue;
		}

	private:
		std::shared_ptr<IAxisInputBinding> m_baseAxisBinding;
		float m_change = 0.0f;
		float m_value = -1.0f;
	};

	class DoubleSwitchAxisInputBinding : public IAxisInputBinding
	{
	public:
		explicit DoubleSwitchAxisInputBinding(
			std::shared_ptr<ISwitchInputBinding> a_upSwitchBinding,
			std::shared_ptr<ISwitchInputBinding> a_downSwitchBinding,
			float const a_restValue = 0.0f,
			float const a_upStrength = 1.0f,
			float const a_downStrength = 1.0f
		)
			: m_upSwitchBinding{ std::move(a_upSwitchBinding) }
			, m_downSwitchBinding{ std::move(a_downSwitchBinding) }
			, m_restValue{ a_restValue }
			, m_upStrength{ a_upStrength }
			, m_downStrength{ a_downStrength }
			, m_value{ m_restValue }
		{
		}

		float getChange() const override
		{
			return m_change;
		}

		float getValue() const override
		{
			return m_value;
		}

		void update(Inputs const& a_inputs, misph::measure_time const a_elapsedTime) override
		{
			m_upSwitchBinding->update(a_inputs, a_elapsedTime);
			m_downSwitchBinding->update(a_inputs, a_elapsedTime);

			auto const oldValue = m_value;
			if (m_upSwitchBinding->isPressed())
			{
				m_value = std::min(1.0f, m_value + a_elapsedTime.get_value() * m_upStrength);
			}
			else if (m_downSwitchBinding->isPressed())
			{
				m_value = std::max(-1.0f, m_value - a_elapsedTime.get_value() * m_downStrength);
			}
			else if (m_value < m_restValue)
			{
				m_value = std::min(
					m_restValue, m_value + a_elapsedTime.get_value() * m_upStrength);
			}
			else
			{
				m_value = std::max(
					m_restValue, m_value - a_elapsedTime.get_value() * m_downStrength);
			}

			m_change = m_value - oldValue;
		}

	private:
		std::shared_ptr<ISwitchInputBinding> m_upSwitchBinding;
		std::shared_ptr<ISwitchInputBinding> m_downSwitchBinding;
		float m_restValue;
		float m_upStrength;
		float m_downStrength;
		float m_change = 0.0f;
		float m_value;
	};

	class SingleSwitchAxisInputBinding : public IAxisInputBinding
	{
	public:
		explicit SingleSwitchAxisInputBinding(
			std::shared_ptr<ISwitchInputBinding> a_switchBinding,
			float const a_upStrength = 1.0f,
			float const a_downStrength = 1.0f
		)
			: m_switchBinding{ a_switchBinding }
			, m_upStrength{ a_upStrength }
			, m_downStrength{ a_downStrength }
		{
		}

		float getChange() const override
		{
			return m_change;
		}

		float getValue() const override
		{
			return m_value;
		}

		void update(Inputs const& a_inputs, misph::measure_time const a_elapsedTime) override
		{
			m_switchBinding->update(a_inputs, a_elapsedTime);

			auto const oldValue = m_value;
			if (m_switchBinding->isPressed())
			{
				m_value = std::min(1.0f, m_value + a_elapsedTime.get_value() * m_upStrength);
			}
			else
			{
				m_value = std::max(-1.0f, m_value - a_elapsedTime.get_value() * m_downStrength);
			}
			m_change = m_value - oldValue;
		}

	private:
		std::shared_ptr<ISwitchInputBinding> m_switchBinding;
		float m_upStrength;
		float m_downStrength;
		float m_change = 0.0f;
		float m_value = -1.0f;
	};
}
