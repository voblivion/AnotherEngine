#pragma once

#include <iostream>

#include <aoe/core/standard/ADynamicType.h>

namespace aoe
{
	namespace data
	{
		using FormatId = std::uint64_t;

		class AFormattedInputStream
			: public sta::ADynamicType
		{
		public:
			// Constructors
			explicit AFormattedInputStream(FormatId const a_formatId)
				: m_formatId{ a_formatId }
			{}

			// Methods
			FormatId getFormat() const
			{
				return m_formatId;
			}

			virtual std::istream& getInputStream() = 0;

		private:
			// Attributes
			FormatId m_formatId;
		};

		template <typename InputStreamType>
		class FormattedInputStream final
			: public AFormattedInputStream
		{
		public:
			// Constructors
			explicit FormattedInputStream(FormatId const a_formatId
				, InputStreamType a_inputStream)
				: AFormattedInputStream{ a_formatId }
				, m_inputStream{ std::move(a_inputStream) }
			{}

			// Methods
			virtual std::istream& getInputStream() override
			{
				return m_inputStream;
			}

		private:
			// Attributes
			InputStreamType m_inputStream;
		};
	}
}
