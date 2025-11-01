#pragma once

#include <vob/aoe/input/InputReference.h>
#include <vob/aoe/input/CommonInputBindings.h>


namespace vob::aoein
{
	namespace InputBindingUtils
	{
		inline auto makeSwitch(SwitchReference const a_switchRef)
		{
			return std::make_shared<DefaultSwitchInputBinding>(a_switchRef);
		}

		inline auto makeSwitch(
			AxisReference const a_axisRef,
			float const a_pressThreshold = 0.0f,
			float const a_releaseThreshold = 0.0f,
			bool const a_isUp = true)
		{
			return std::make_shared<AxisSwitchInputBinding>(
				std::make_shared<DefaultAxisInputBinding>(a_axisRef), a_pressThreshold, a_releaseThreshold, a_isUp);
		}

		inline auto makeAxis(
			AxisReference const a_axisRef,
			float const a_sensitivity = 1.0f,
			float const a_deadZoneValue = 0.0f,
			float const a_deadZoneRadius = 0.0f)
		{
			return std::make_shared<DefaultAxisInputBinding>(
				a_axisRef, a_sensitivity, a_deadZoneValue, a_deadZoneRadius);
		}

		inline auto makeDerivedAxis(AxisReference const a_axisRef, float const a_sensitivity = 1.0f)
		{
			return std::make_shared<DerivedAxisInputBinding>(
				std::make_shared<DefaultAxisInputBinding>(a_axisRef, a_sensitivity));
		}

		inline auto makeAxis(
			SwitchReference const a_switchRef,
			float const a_upStrength = 1000.0f,
			float const a_downStrength = 1000.0f)
		{
			return std::make_shared<SingleSwitchAxisInputBinding>(
				std::make_shared<DefaultSwitchInputBinding>(a_switchRef), a_upStrength, a_downStrength);
		}

		inline auto makeAxis(
			SwitchReference const a_switchUpRef,
			SwitchReference const a_switchDownRef,
			float const a_restValue = 0.0f,
			float const a_upStrength = 1000.0f,
			float const a_downStrength = 1000.0f)
		{
			return std::make_shared<DoubleSwitchAxisInputBinding>(
				std::make_shared<DefaultSwitchInputBinding>(a_switchUpRef),
				std::make_shared<DefaultSwitchInputBinding>(a_switchDownRef),
				a_restValue,
				a_upStrength,
				a_downStrength);
		}
	}
}
