#include <vob/aoe/storage/SettingsStore.h>


namespace vob::aoesg
{
	SettingsStore::SettingsStore(IStorage& a_storage, std::string a_name)
		: m_storage{ a_storage }
		, m_name{ std::move(a_name) }
	{}

	bool SettingsStore::load()
	{
		for (auto& entry : m_pages)
		{
			entry.second->reset();
		}

		auto stream = m_storage.openForRead(StorageDomain::Settings, m_name);
		if (stream == nullptr)
		{
			return false;
		}

		auto document = mistd::ini_object{};
		*stream >> document;

		ReaderType reader{ m_readApplicator, std::monostate{} };
		for (auto& entry : m_pages)
		{
			auto& page = *entry.second;
			auto const sectionIt = document.data.find(std::string_view{ page.name });
			if (sectionIt == document.data.end())
			{
				continue;
			}

			VOB_AOE_CHECK_LOG(
				page.read(reader, sectionIt->second),
				"Some values of settings section [{}] could not be read, using defaults.",
				page.name);
		}
		return true;
	}

	bool SettingsStore::save()
	{
		auto document = mistd::ini_object{};
		WriterType writer{ m_writeApplicator, std::monostate{} };
		auto result = true;
		for (auto& entry : m_pages)
		{
			auto& page = *entry.second;
			document.data.emplace(
				mistd::ini_object::string_type{ page.name }, mistd::ini_value{});
			auto const sectionIt = document.data.find(std::string_view{ page.name });
			result &= page.write(writer, sectionIt->second);
		}

		auto stream = m_storage.openForWrite(StorageDomain::Settings, m_name);
		if (stream == nullptr)
		{
			return false;
		}

		*stream << document;
		return result;
	}
}
