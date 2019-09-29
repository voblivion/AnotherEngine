#pragma once

#include <iostream>

#include <aoe/core/standard/ADynamicType.h>

namespace aoe::data
{
	using FormatId = std::uint64_t;

	template <typename InputStreamType>
	class FormattedInputStream final
	{
	public:
		// Constructors
		explicit FormattedInputStream(FormatId const a_formatId
			, InputStreamType a_inputStream)
			: m_formatId{ a_formatId }
			, m_inputStream{ std::move(a_inputStream) }
		{}

		// Methods
		FormatId getFormat() const
		{
			return m_formatId;
		}

		std::istream& getInputStream()
		{
			return m_inputStream;
		}

	private:
		// Attributes
		FormatId m_formatId;
		InputStreamType m_inputStream;
	};
}
