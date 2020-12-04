#pragma once

#include <memory>

#include <vob/aoe/core/type/ADynamicType.h>

#include <vob/aoe/common/data/filesystem/FileSystem.h>

namespace vob::aoe::common
{
	class VOB_AOE_API AFileSystemLoader
		: public type::ADynamicType
	{
	public:
		// Methods
		virtual bool canLoad(std::filesystem::path const& a_path) const = 0;

		virtual std::shared_ptr<type::ADynamicType> load(std::filesystem::path const& a_path) const = 0;
	};
}
