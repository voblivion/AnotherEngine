#pragma once

#include <vob/aoe/core/type/ADynamicType.h>

#include <string>


namespace vob::aoe::common
{
	struct Text final
		: type::ADynamicType
	{
		template <typename... Args>
		explicit Text(Args&&... a_args)
			: m_string{ std::forward<Args>(a_args)... }
		{}

		std::pmr::string m_string;
	};
}