#pragma once

#include <filesystem>
#include <functional>
#include <set>
#include <typeindex>
#include <unordered_map>
#include <utility>


namespace vob::aoedt
{
	template <typename TCollector>
	concept DependencyCollector = requires(
		TCollector const& a_collector,
		std::filesystem::path const& a_path,
		std::set<std::pair<std::type_index, std::filesystem::path>>& o_dependencies)
	{
		a_collector.collectDependencies(a_path, o_dependencies);
	};

	class MultiDependencyCollector
	{
	public:
		template <typename TData, DependencyCollector TCollector>
		void registerDependencyCollector(TCollector const& a_collector)
		{
			m_collectors.try_emplace(
				typeid(TData),
				[&a_collector](
					std::filesystem::path const& a_path,
					std::set<std::pair<std::type_index, std::filesystem::path>>& o_dependencies)
				{
					a_collector.collectDependencies(a_path, o_dependencies);
				});
		}

		template <typename TData>
		void collectDependencies(
			std::filesystem::path const& a_path,
			std::set<std::pair<std::type_index, std::filesystem::path>>& o_dependencies) const
		{
			auto const path = a_path.lexically_normal();
			if (!o_dependencies.emplace(typeid(TData), path).second)
			{
				return;
			}

			auto const collectorIt = m_collectors.find(typeid(TData));
			if (collectorIt == m_collectors.end())
			{
				return;
			}

			collectorIt->second(path, o_dependencies);
		}

	private:
		std::unordered_map<
			std::type_index,
			std::function<void(
				std::filesystem::path const&, std::set<std::pair<std::type_index, std::filesystem::path>>&)>>
			m_collectors;
	};
}
