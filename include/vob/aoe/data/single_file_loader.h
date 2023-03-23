#pragma once

#include <fstream>
#include <utility>


namespace vob::aoedt
{
	template <typename TStreamLoader>
	class single_file_loader
	{
	public:
		explicit single_file_loader(TStreamLoader&& a_loader)
			: m_streamLoader{ a_loader }
		{}

		template <typename... TArgs>
		explicit single_file_loader(TArgs&&... a_args)
			: m_streamLoader{ std::forward<TArgs>(a_args)... }
		{}

		auto load(std::filesystem::path const& a_path) const
		{
			auto file = std::ifstream{ a_path, std::ios::binary | std::ios::in };
			return m_streamLoader.load(file);
		}

		TStreamLoader m_streamLoader;
	};
}
