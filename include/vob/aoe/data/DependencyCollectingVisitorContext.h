#pragma once

#include <vob/aoe/data/MultiDependencyCollector.h>

#include <vob/misc/type/factory.h>
#include <vob/misc/visitor/json_reader.h>

#include <filesystem>
#include <set>
#include <typeindex>
#include <utility>


namespace vob::aoedt
{
	class DependencyCollectingVisitorContext
	{
	public:
		DependencyCollectingVisitorContext(
			misty::pmr::factory const& a_factory,
			MultiDependencyCollector const& a_multiDependencyCollector,
			std::set<std::pair<std::type_index, std::filesystem::path>>& o_dependencies,
			std::filesystem::path a_basePath)
			: m_factory{ a_factory }
			, m_multiDependencyCollector{ a_multiDependencyCollector }
			, m_dependencies{ o_dependencies }
			, m_basePath{ std::move(a_basePath) }
		{}

		auto const& getFactory() const
		{
			return m_factory;
		}

		auto const& getMultiDependencyCollector() const
		{
			return m_multiDependencyCollector;
		}

		auto& getDependencies()
		{
			return m_dependencies;
		}

		auto const& getBasePath() const
		{
			return m_basePath;
		}

	private:
		misty::pmr::factory const& m_factory;
		MultiDependencyCollector const& m_multiDependencyCollector;
		std::set<std::pair<std::type_index, std::filesystem::path>>& m_dependencies;
		std::filesystem::path m_basePath;
	};
}

namespace vob::misvi
{
	template <typename... TArgs>
	auto const& get_factory(json_reader<aoedt::DependencyCollectingVisitorContext, TArgs...> const& a_visitor)
	{
		return a_visitor.get_context().getFactory();
	}
}
