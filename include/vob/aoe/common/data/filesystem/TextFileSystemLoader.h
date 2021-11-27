#pragma once

#include <fstream>

#include <vob/aoe/common/data/filesystem/AFileSystemLoader.h>
#include <vob/aoe/common/data/filesystem/Text.h>

namespace vob::aoe::common
{
	class TextFileSystemLoader final
		: public AFileSystemLoader
	{
	public:
#pragma region Methods
		bool canLoad(std::filesystem::path const&) const override
		{
			return true;
		}

		std::shared_ptr<type::ADynamicType> load(std::filesystem::path const& a_path) const override
		{
			auto file = std::ifstream{ a_path, std::ios::binary | std::ios::in };

			return std::make_unique<Text>(
				std::istreambuf_iterator<char>(file)
				, std::istreambuf_iterator<char>{}
			);
		}
#pragma endregion
	};
}
