#pragma once

#include "vob/aoe/debug/Check.h"

#include <vob/misc/visitor/json_reader.h>

#include <filesystem>
#include <format>
#include <fstream>


namespace vob::aoedt
{
	template <typename TData>
	struct DefaultConstructorFactory
	{
		TData operator()() const
		{
			return {};
		}
	};

	template <
		typename TData
		, typename TContextFactory
		, typename TDataFactory = DefaultConstructorFactory<TData>>
	class JsonFileLoader
	{
		using ContextType = decltype(
			std::declval<TContextFactory const>()(std::declval<std::filesystem::path const&>()));

	public:
		explicit JsonFileLoader(
			misvi::pmr::applicator<
				false, misvi::pmr::json_reader<ContextType>> const& a_applicator,
			TContextFactory a_contextFactory,
			TDataFactory a_factory = {})
			: m_factory{ std::forward<TDataFactory>(a_factory) }
			, m_applicator{ a_applicator }
			, m_contextFactory{ std::forward<TContextFactory>(a_contextFactory) }
		{}

		TData load(std::filesystem::path const& a_path) const
		{
			auto file = std::ifstream{ a_path, std::ios::binary | std::ios::in };
			if (!VOB_AOE_CHECK_LOG(file.is_open(), "Failed to open file: {}.", a_path.generic_string()))
			{
				return m_factory();
			}

			mistd::pmr::json_value jsonValue;
			file >> jsonValue;
			file.close();

			TData data = m_factory();
			auto context = m_contextFactory(a_path);
			misvi::pmr::json_reader<ContextType> reader{ m_applicator, context };
			reader.read(jsonValue, data);
			return data;
		}

	private:
		TDataFactory m_factory;
		misvi::pmr::applicator<false, misvi::pmr::json_reader<ContextType>> const& m_applicator;
		TContextFactory m_contextFactory;
	};
}
