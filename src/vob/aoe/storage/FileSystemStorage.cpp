#include <vob/aoe/storage/FileSystemStorage.h>

#include <sago/platform_folders.h>

#include <fstream>


namespace vob::aoesg
{
	FileSystemStorage::FileSystemStorage(
		std::string_view a_organizationName, std::string_view a_applicationName)
	{
		auto const applicationDirectory =
			std::filesystem::path{ sago::getConfigHome() } / a_organizationName / a_applicationName;
		m_settingsDirectory = applicationDirectory / "settings";
		m_savesDirectory =
			std::filesystem::path{ sago::getSaveGamesFolder1() } / a_organizationName / a_applicationName;
	}

	std::unique_ptr<std::istream> FileSystemStorage::openForRead(
		StorageDomain a_domain, std::string_view a_name) const
	{
		auto stream = std::make_unique<std::ifstream>(
			getDirectory(a_domain) / a_name, std::ios::binary);
		return stream->is_open() ? std::move(stream) : nullptr;
	}

	std::unique_ptr<std::ostream> FileSystemStorage::openForWrite(
		StorageDomain a_domain, std::string_view a_name)
	{
		auto const& directory = getDirectory(a_domain);
		auto errorCode = std::error_code{};
		std::filesystem::create_directories(directory, errorCode);

		auto stream = std::make_unique<std::ofstream>(directory / a_name, std::ios::binary);
		return stream->is_open() ? std::move(stream) : nullptr;
	}

	std::filesystem::path const& FileSystemStorage::getDirectory(StorageDomain a_domain) const
	{
		return a_domain == StorageDomain::Saves ? m_savesDirectory : m_settingsDirectory;
	}
}
