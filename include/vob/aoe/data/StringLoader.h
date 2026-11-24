#pragma once

#include <iostream>
#include <string>


namespace vob::aoedt
{
	class StringLoader
	{
	public:
		std::pmr::string load(std::istream& a_stream) const
		{
			return { std::istreambuf_iterator<char>(a_stream), std::istreambuf_iterator<char>{} };
		}
	};
}
