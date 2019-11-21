#pragma once

#include <istream>
#include <vector>


namespace vob::aoe::data
{
	inline std::size_t getSize(std::istream& a_inputStream)
	{
		auto const t_startPos = a_inputStream.tellg();
		a_inputStream.seekg(0, std::ios::end);
		auto const t_endPos = a_inputStream.tellg();
		a_inputStream.seekg(t_startPos, std::ios::beg);

		return t_endPos - t_startPos;
	}

	inline std::pmr::vector<char> getData(std::istream& a_inputStream)
	{
		auto const t_size = getSize(a_inputStream);

		// Copy stream into vector
		std::pmr::vector<char> t_bytes;
		t_bytes.resize(t_size);
		if (t_bytes.empty())
		{
			return {};
		}
		a_inputStream.read(&t_bytes[0], t_bytes.size());

		return t_bytes;
	}
}
