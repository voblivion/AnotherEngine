#pragma once

#include <iostream>
#include <string>


namespace vob::aoedt
{
	class string_loader
	{
	public:
		std::pmr::string load(std::istream& a_stream) const
		{
			return { std::istreambuf_iterator<char>(a_stream), std::istreambuf_iterator<char>{} };
		}
	};
}
