#include <vob/aoe/exchange/EcsExchangeData.h>


namespace vob::aoexc
{
	entt::registry& EcsExchangeData::beginStore()
	{
		return m_registries[m_writeRegistryIndex].first;
	}

	void EcsExchangeData::endStore(EventPool& a_eventPool)
	{
		std::lock_guard lock(m_mutex);
		m_registries[m_writeRegistryIndex].second = true;
		std::swap(m_writeRegistryIndex, m_freeRegistryIndex);
		m_eventPools[1 - m_readEventPoolIndex].merge(a_eventPool);
	}

	std::optional<std::pair<entt::registry const*, EventPool const*>> EcsExchangeData::tryBeginLoad()
	{
		std::lock_guard lock(m_mutex);
		if (!m_registries[m_freeRegistryIndex].second)
		{
			return std::nullopt;
		}
		std::swap(m_readRegistryIndex, m_freeRegistryIndex);
		m_readEventPoolIndex = 1 - m_readEventPoolIndex;
		return std::make_pair(&m_registries[m_readRegistryIndex].first, &m_eventPools[m_readEventPoolIndex]);
	}

	void EcsExchangeData::endLoad()
	{
		std::lock_guard lock(m_mutex);
		m_registries[m_readRegistryIndex].second = false;
	}
}
