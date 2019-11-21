#pragma once

#include <iostream>
#include <vob/sta/string_id.h>


namespace vob::aoe::data
{
	using FormatId = sta::string_id;

	template <typename InputStreamType>
	class FormattedInputStream final
	{
	public:
		// Constructors
		explicit FormattedInputStream(
			FormatId const a_formatId
			, InputStreamType a_inputStream
		)
			: m_formatId{ a_formatId }
			, m_inputStream{ std::move(a_inputStream) }
		{}

		// Methods
		FormatId getFormat() const
		{
			return m_formatId;
		}

		InputStreamType& getInputStream()
		{
			return m_inputStream;
		}

	private:
		// Attributes
		FormatId m_formatId;
		InputStreamType m_inputStream;
	};
}
