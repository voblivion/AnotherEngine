#pragma once

#include <vob/aoe/storage/IStorage.h>

#include <filesystem>
#include <string>


namespace vob::aoesg
{
	class FileSystemStorage final : public IStorage
	{
	public:
		FileSystemStorage(
			std::string_view a_organizationName, std::string_view a_applicationName);

		std::unique_ptr<std::istream> openForRead(
			StorageDomain a_domain, std::string_view a_name) const override;

		std::unique_ptr<std::ostream> openForWrite(
			StorageDomain a_domain, std::string_view a_name) override;

		std::filesystem::path const& getDirectory(StorageDomain a_domain) const;

	private:
		std::filesystem::path m_settingsDirectory;
		std::filesystem::path m_savesDirectory;
	};
}
