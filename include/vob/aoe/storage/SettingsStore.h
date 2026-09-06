#pragma once

#include <vob/aoe/storage/IStorage.h>

#include <vob/misc/visitor/ini_reader.h>
#include <vob/misc/visitor/ini_writer.h>

#include <memory>
#include <string>
#include <variant>
#include <vector>


namespace vob::aoesg
{
	class SettingsStore
	{
	public:
		explicit SettingsStore(IStorage& a_storage, std::string a_name = "settings.ini");

		template <typename TSettings>
		TSettings& add(std::string a_name)
		{
			auto page = std::make_unique<SettingsPage<TSettings>>(std::move(a_name));
			auto& value = page->get();
			m_pages.push_back(std::move(page));
			return value;
		}

		bool load();
		bool save();

	private:
		using ReaderType = misvi::ini_reader<std::monostate>;
		using WriterType = misvi::ini_writer<std::monostate>;

		struct ASettingsPage
		{
			explicit ASettingsPage(std::string a_name)
				: name{ std::move(a_name) }
			{}
			virtual ~ASettingsPage() = default;

			virtual bool read(ReaderType& a_reader, mistd::ini_value const& a_iniValue) = 0;
			virtual bool write(WriterType& a_writer, mistd::ini_value& a_iniValue) const = 0;

			std::string name;
		};

		template <typename TSettings>
		class SettingsPage final : public ASettingsPage
		{
		public:
			using ASettingsPage::ASettingsPage;

			bool read(ReaderType& a_reader, mistd::ini_value const& a_iniValue) override
			{
				return a_reader.read(a_iniValue, m_value);
			}

			bool write(WriterType& a_writer, mistd::ini_value& a_iniValue) const override
			{
				return a_writer.write(a_iniValue, m_value);
			}

			TSettings& get()
			{
				return m_value;
			}

		private:
			TSettings m_value;
		};

		IStorage& m_storage;
		std::string m_name;
		misvi::applicator<false, ReaderType> m_readApplicator;
		misvi::applicator<true, WriterType> m_writeApplicator;
		std::vector<std::unique_ptr<ASettingsPage>> m_pages;
	};
}
