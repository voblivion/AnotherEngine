#pragma once

#include <vob/aoe/input/common_bindings.h>


namespace vob::aoein
{
	namespace binding_util
	{
		auto make_switch(switch_reference const a_switchRef)
		{
			return std::make_shared<default_switch_binding>(a_switchRef);
		}

		auto make_switch(
			axis_reference const a_axisRef,
			float const a_pressThreshold = 0.0f,
			float const a_releaseThreshold = 0.0f,
			bool const a_isUp = true)
		{
			return std::make_shared<axis_switch_binding>(
				std::make_shared<default_axis_binding>(a_axisRef), a_pressThreshold, a_releaseThreshold, a_isUp);
		}

		auto make_axis(
			axis_reference const a_axisRef,
			float const a_sensitivity = 1.0f,
			float const a_deadZoneValue = 0.0f,
			float const a_deadZoneRadius = 0.0f)
		{
			return std::make_shared<default_axis_binding>(
				a_axisRef, a_sensitivity, a_deadZoneValue, a_deadZoneRadius);
		}

		auto make_derived_axis(axis_reference const a_axisRef, float const a_sensitivity = 1.0f)
		{
			return std::make_shared<derived_axis_binding>(
				std::make_shared<default_axis_binding>(a_axisRef, a_sensitivity));
		}

		auto make_axis(
			switch_reference const a_switchRef,
			float const a_upStrength = 1000.0f,
			float const a_downStrength = 1000.0f)
		{
			return std::make_shared<single_switch_axis_binding>(
				std::make_shared<default_switch_binding>(a_switchRef), a_upStrength, a_downStrength);
		}

		auto make_axis(
			switch_reference const a_switchUpRef,
			switch_reference const a_switchDownRef,
			float const a_restValue = 0.0f,
			float const a_upStrength = 1000.0f,
			float const a_downStrength = 1000.0f)
		{
			return std::make_shared<double_switch_axis_binding>(
				std::make_shared<default_switch_binding>(a_switchUpRef),
				std::make_shared<default_switch_binding>(a_switchDownRef),
				a_restValue,
				a_upStrength,
				a_downStrength);
		}
	}
}
