#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/exchange/EventPool.h>

#include <entt/entt.hpp>

#include <array>
#include <mutex>


namespace vob::aoexc
{
	class VOB_AOE_API EcsExchangeData
	{
	public:
		entt::registry& beginStore();
		void endStore(EventPool& a_eventPool);

		std::optional<std::pair<entt::registry const*, EventPool const*>> tryBeginLoad();
		void endLoad();

	private:
		std::mutex m_mutex;
		int32_t m_readRegistryIndex = 0;
		int32_t m_writeRegistryIndex = 1;
		int32_t m_freeRegistryIndex = 2;
		std::array<std::pair<entt::registry, bool>, 3> m_registries;
		int32_t m_readEventPoolIndex = 0;
		std::array<EventPool, 2> m_eventPools;
	};

}
