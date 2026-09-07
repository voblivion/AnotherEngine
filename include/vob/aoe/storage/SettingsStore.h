#pragma once

#include <vob/aoe/debug/Check.h>
#include <vob/aoe/storage/IStorage.h>

#include <vob/misc/std/vector_map.h>
#include <vob/misc/visitor/ini_reader.h>
#include <vob/misc/visitor/ini_writer.h>

#include <memory>
#include <string>
#include <typeindex>
#include <variant>


namespace vob::aoesg
{
	class SettingsStore
	{
	public:
		explicit SettingsStore(IStorage& a_storage, std::string a_name = "settings.ini");
		SettingsStore(SettingsStore const&) = delete;
		SettingsStore(SettingsStore&&) = delete;
		~SettingsStore() = default;

		SettingsStore& operator=(SettingsStore const&) = delete;
		SettingsStore& operator=(SettingsStore&&) = delete;

		template <typename TSettings>
		TSettings& add(std::string a_name)
		{
			auto const typeIndex = std::type_index{ typeid(TSettings) };
			VOB_AOE_CHECK_TERMINATE(
				m_pages.find(typeIndex) == m_pages.end(),
				"Settings page {} is already registered.",
				typeIndex.name());

			auto page = std::make_unique<SettingsPage<TSettings>>(std::move(a_name));
			auto& value = page->get();
			m_pages.emplace(typeIndex, std::move(page));
			return value;
		}

		template <typename TSettings>
		TSettings& get()
		{
			auto const typeIndex = std::type_index{ typeid(TSettings) };
			auto const pageIt = m_pages.find(typeIndex);
			VOB_AOE_CHECK_TERMINATE(
				pageIt != m_pages.end(), "Settings store is missing a {} page.", typeIndex.name());

			return static_cast<SettingsPage<TSettings>*>(pageIt->second.get())->get();
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

			virtual void reset() = 0;
			virtual bool read(ReaderType& a_reader, mistd::ini_value const& a_iniValue) = 0;
			virtual bool write(WriterType& a_writer, mistd::ini_value& a_iniValue) const = 0;

			std::string name;
		};

		template <typename TSettings>
		class SettingsPage final : public ASettingsPage
		{
		public:
			using ASettingsPage::ASettingsPage;

			void reset() override
			{
				m_value = TSettings{};
			}

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
		mistd::vector_map<std::type_index, std::unique_ptr<ASettingsPage>> m_pages;
	};
}
