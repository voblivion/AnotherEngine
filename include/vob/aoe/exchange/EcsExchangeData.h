#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/exchange/EventPool.h>

#include <entt/entt.hpp>

#include <mutex>


namespace vob::aoexc
{
	class VOB_AOE_API EcsExchangeData
	{
	public:
		void store(entt::registry a_registry, EventPool a_eventPool);

		std::pair<entt::registry, EventPool> load();

	private:
		std::mutex m_mutex;
		entt::registry m_registry;
		// TODO: if multiple readers, there should be multiple pools, but will there ever be multiple readers?
		EventPool m_eventPool;
	};

}
