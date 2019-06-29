#pragma once

#include <iostream>
#include <SFML/System/InputStream.hpp>

namespace vob
{
	class SfmlInputStream final
		: public sf::InputStream
	{
	public:
		// Constructors
		explicit SfmlInputStream(std::istream& a_inputStream)
			: m_inputStream{ a_inputStream }
			, m_startPos{ a_inputStream.tellg() }
		{}

		// Methods
		virtual sf::Int64 read(void* a_data, sf::Int64 a_size) final override
		{
			m_inputStream.read(static_cast<char*>(a_data), a_size);
			return tell();
		}

		virtual sf::Int64 seek(sf::Int64 a_position) final override
		{
			if (m_inputStream.bad() || m_inputStream.fail())
			{
				return -1;
			}
			m_inputStream.seekg(m_startPos + a_position);
			return tell();

		}

		virtual sf::Int64 tell() final override
		{
			if (m_inputStream.bad() || m_inputStream.fail())
			{
				return -1;
			}
			return m_inputStream.tellg() - m_startPos;
		}

		virtual sf::Int64 getSize() final override
		{
			if (m_inputStream.bad() || m_inputStream.fail())
			{
				return -1;
			}
			auto const t_currentPos = m_inputStream.tellg();
			m_inputStream.seekg(0, std::ios::end);
			auto const t_endPos = m_inputStream.tellg();
			m_inputStream.seekg(t_currentPos);
			return t_endPos - m_startPos;
		}

	private:
		// Attributes
		std::istream& m_inputStream;
		std::char_traits<char>::pos_type m_startPos;
	};
}