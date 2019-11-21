#pragma once

#include <optional>

#include <vob/aoe/common/space/Vector.h>

namespace vob::aoe::common::gui
{
	struct ARule
	{
		virtual ~ARule() = default;

		virtual float value(Vector2 const& a_viewSize) const = 0;

		void tryApply(
			std::optional<float>& a_value
			, common::Vector2 const& a_viewSize
		) const
		{
			if (!a_value.has_value())
			{
				a_value = value(a_viewSize);
			}
		}
	};
}
