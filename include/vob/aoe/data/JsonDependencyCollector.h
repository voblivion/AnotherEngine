#pragma once

#include "vob/aoe/debug/Check.h"

#include <vob/aoe/data/DependencyCollectingVisitorContext.h>
#include <vob/aoe/data/JsonAccept.h>
#include <vob/aoe/data/JsonFileLoader.h>
#include <vob/aoe/data/MultiDependencyCollector.h>

#include <vob/misc/type/factory.h>
#include <vob/misc/visitor/json_reader.h>

#include <filesystem>
#include <format>
#include <fstream>
#include <set>
#include <typeindex>
#include <utility>


namespace vob::aoedt
{
	template <typename TData, typename TDataFactory = DefaultConstructorFactory<TData>>
	class JsonDependencyCollector
	{
	public:
		JsonDependencyCollector(
			misvi::pmr::applicator<
				false, misvi::pmr::json_reader<DependencyCollectingVisitorContext>> const& a_applicator,
			misty::pmr::factory const& a_factory,
			MultiDependencyCollector const& a_multiDependencyCollector,
			TDataFactory a_dataFactory = {})
			: m_applicator{ a_applicator }
			, m_factory{ a_factory }
			, m_multiDependencyCollector{ a_multiDependencyCollector }
			, m_dataFactory{ std::move(a_dataFactory) }
		{}

		void collectDependencies(
			std::filesystem::path const& a_path,
			std::set<std::pair<std::type_index, std::filesystem::path>>& o_dependencies) const
		{
			auto file = std::ifstream{ a_path, std::ios::binary | std::ios::in };
			if (!VOB_AOE_CHECK_LOG(file.is_open(), "Failed to open file: {}.", a_path.generic_string()))
			{
				return;
			}

			mistd::pmr::json_value jsonValue;
			file >> jsonValue;
			file.close();

			TData data = m_dataFactory();
			auto context = DependencyCollectingVisitorContext{
				m_factory, m_multiDependencyCollector, o_dependencies, a_path };
			misvi::pmr::json_reader<DependencyCollectingVisitorContext> reader{ m_applicator, context };
			reader.read(jsonValue, data);
		}

	private:
		misvi::pmr::applicator<false, misvi::pmr::json_reader<DependencyCollectingVisitorContext>> const&
			m_applicator;
		misty::pmr::factory const& m_factory;
		MultiDependencyCollector const& m_multiDependencyCollector;
		TDataFactory m_dataFactory;
	};
}
