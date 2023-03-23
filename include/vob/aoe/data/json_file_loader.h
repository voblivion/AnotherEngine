#pragma once

#include <vob/misc/visitor/json_reader.h>

#include <fstream>


namespace vob::aoedt
{
	template <typename TData>
	struct default_constructor_factory
	{
		TData operator()() const
		{
			return {};
		}
	};

	template <
		typename TData
		, typename TContextFactory
		, typename TDataFactory = default_constructor_factory<TData>>
	class json_file_loader
	{
		using context_type = decltype(
			std::declval<TContextFactory const>()(std::declval<std::filesystem::path const&>()));

	public:
		explicit json_file_loader(
			misvi::pmr::applicator<
				false, misvi::pmr::json_reader<context_type>> const& a_applicator,
			TContextFactory a_contextFactory,
			TDataFactory a_factory = {})
			: m_factory{ std::forward<TDataFactory>(a_factory) }
			, m_applicator{ a_applicator }
			, m_contextFactory{ std::forward<TContextFactory>(a_contextFactory) }
		{}

		auto load(std::filesystem::path const& a_path) const
		{
			auto file = std::ifstream{ a_path, std::ios::binary | std::ios::in };
			mistd::pmr::json_value jsonValue;
			file >> jsonValue;
			file.close();

			TData data = m_factory();
			auto context = m_contextFactory(a_path);
			misvi::pmr::json_reader<context_type> reader{ m_applicator, context };
			reader.read(jsonValue, data);
			return data;
		}

	private:
		TDataFactory const& m_factory;
		misvi::pmr::applicator<false, misvi::pmr::json_reader<context_type>> const& m_applicator;
		TContextFactory m_contextFactory;
	};
}
