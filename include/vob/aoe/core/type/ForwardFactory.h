#pragma once

#include <utility>


namespace vob::aoe::type
{
	template <typename Type, typename... Args>
	class ForwardFactory final
	{
	public:
		// Constructors
		explicit ForwardFactory(Args... a_args)
			: m_args{ std::forward<Args>(a_args)... }
		{}

		// Methods
		Type operator()()
		{
			return impl(std::index_sequence_for<Args...>{});
		}

	private:
		// Attributes
		std::tuple<Args...> m_args;

		// Methods
		template <std::size_t... indexes>
		Type impl(std::index_sequence<indexes...>)
		{
			return Type{ std::forward<Args>(std::get<indexes>(m_args))... };
		}
	};
}