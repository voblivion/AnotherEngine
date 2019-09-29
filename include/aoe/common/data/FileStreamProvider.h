#pragma once

#include <optional>
#include <aoe/common/data/FileIndexer.h>
#include <aoe/core/data/FormattedInputStream.h>

namespace aoe::common
{
	class FileStreamProvider
	{
	public:
		// Constructor
		explicit FileStreamProvider(FileIndexer const& a_fileIndexer)
			: m_fileIndexer{ a_fileIndexer }
		{}

		// Methods
		std::optional<data::FormattedInputStream<std::ifstream>> find(
			data::Id const a_dataId) const
		{
			using InputStream = data::FormattedInputStream<std::ifstream>;

			if (auto const t_formattedFile = m_fileIndexer.find(a_dataId))
			{
				auto const t_mode = t_formattedFile->isBinary()
					? std::ios::binary | std::ios::in : std::ios::in;
				return InputStream{ t_formattedFile->getFormat()
					, std::ifstream{ t_formattedFile->getName().data(), t_mode } };
			}
			return {};
		}

	private:
		// Attributes
		FileIndexer const& m_fileIndexer;
	};
}
