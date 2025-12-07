#include <vob/aoe/exchange/EcsExchangeData.h>


namespace vob::aoexc
{
	void EcsExchangeData::store(entt::registry a_registry, EventPool a_eventPool)
	{
		std::lock_guard lock(m_mutex);
		std::swap(m_registry, a_registry);
		m_eventPool.merge(std::move(a_eventPool));
	}

	std::pair<entt::registry, EventPool> EcsExchangeData::load()
	{
		entt::registry registry;
		EventPool eventPool;

		std::lock_guard lock(m_mutex);
		std::swap(registry, m_registry);
		std::swap(eventPool, m_eventPool);
		return std::make_pair(std::move(registry), std::move(eventPool));
	}
}
