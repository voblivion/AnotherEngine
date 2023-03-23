#pragma once

#include <tuple>

#include <vob/aoe/ecs/world_data_provider.h>

namespace vob::aoe::common
{
	template <typename... TSteps>
	class MultiStepRenderPass
	{
	public:
		// Constructor
		explicit MultiStepRenderPass(aoecs::world_data_provider& a_wdp)
			: m_steps{ TSteps{ a_wdp }... }
		{}

		// Methods
		void run() const
		{
			runSteps(m_steps);
		}

	private:
		// Attributes
		std::tuple<TSteps...> m_steps;

		// Methods
		template <std::size_t t_index = 0, typename... TTypes>
		std::enable_if_t<t_index == sizeof...(TSteps)> runSteps(std::tuple<TTypes...> const& a_tuple) const
		{
			
		}
		template <std::size_t t_index = 0, typename... TTypes>
		std::enable_if_t<t_index < sizeof...(TSteps)> runSteps(std::tuple<TTypes...> const& a_tuple) const
		{
			if (std::get<t_index>(a_tuple).run())
			{
				runSteps<t_index + 1>(a_tuple);
			}
		}
	};
}
