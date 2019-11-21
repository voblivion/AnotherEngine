#pragma once

#include <optional>

namespace vob::aoe::common::gui
{
	struct Constraint
	{
		std::optional<float> m_x = std::nullopt;
		std::optional<float> m_y = std::nullopt;
		std::optional<float> m_width = std::nullopt;
		std::optional<float> m_height = std::nullopt;
	};
}