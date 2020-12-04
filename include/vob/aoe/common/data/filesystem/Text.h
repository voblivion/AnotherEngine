#pragma once

#include <vob/aoe/core/type/Primitive.h>

#include <vob/aoe/core/type/ADynamicType.h>

namespace vob::aoe::common
{
	struct Text final
		: type::ADynamicType
	{
		template <typename... Args>
		explicit Text(Args&&... a_args)
			: m_string{ std::forward<Args>(a_args)... }
		{}

		u8string m_string;
	};
}