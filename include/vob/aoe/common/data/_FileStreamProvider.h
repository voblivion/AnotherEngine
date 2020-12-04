#pragma once

#include <fstream>
#include <vob/aoe/common/data/ResourcePathIndexer.h>

namespace vob::aoe::common
{
	namespace fs = std::filesystem;

	class FileStreamProvider
	{
	public:
		// Alias
		using InputStream = std::ifstream;

		// Constructor
		explicit FileStreamProvider(ResourcePathIndexer& a_resourcePathIndexer)
			: m_resourcePathIndexer{ a_resourcePathIndexer }
		{}

		// Methods
		bool find(data::Id const a_dataId, InputStream& a_is) const
		{
			if (auto const t_resourcePath = m_resourcePathIndexer.findResourcePath(a_dataId))
			{
				a_is.open(t_resourcePath->value().data(), std::ios::binary | std::ios::in);
				return a_is.is_open();
			}
			return false;
		}

	private:
		// Attributes
		ResourcePathIndexer& m_resourcePathIndexer;
	};
}
