#pragma once

#include "vob/aoe/debug/Check.h"

#include <entt/entt.hpp>

#include <vector>


namespace vob::aoeng
{
	struct EcsEntitySyncContext
	{
	public:
		entt::entity tryGetOurEntity(entt::entity a_theirEntity) const
		{
			if (entt::to_entity(a_theirEntity) < static_cast<size_t>(m_staticEntityCount))
			{
				VOB_AOE_CHECK_TERMINATE_SLOW(entt::to_version(a_theirEntity) == 0, "Static entity %s was destroyed.", entt::to_entity(a_theirEntity));
				return a_theirEntity;
			}

			auto const offset = static_cast<size_t>(entt::to_entity(a_theirEntity)) - static_cast<size_t>(m_staticEntityCount);
			if (offset < m_toOurs.size())
			{
				if (m_toOurs[offset].version == entt::to_version(a_theirEntity))
				{
					return m_toOurs[offset].mappedEntity;
				}
			}

			return entt::null;
		}

		entt::entity tryGetTheirEntity(entt::entity a_ourEntity) const
		{
			if (entt::to_entity(a_ourEntity) < static_cast<size_t>(m_staticEntityCount))
			{
				VOB_AOE_CHECK_TERMINATE_SLOW(entt::to_version(a_ourEntity) == 0, "Static entity %s was destroyed.", entt::to_entity(a_ourEntity));
				return a_ourEntity;
			}

			auto const offset = static_cast<size_t>(entt::to_entity(a_ourEntity)) - static_cast<size_t>(m_staticEntityCount);
			if (offset < m_toTheirs.size())
			{
				if (m_toTheirs[offset].version == entt::to_version(a_ourEntity))
				{
					return m_toTheirs[offset].mappedEntity;
				}
			}

			return entt::null;
		}

		void setStaticEntityCount(size_t a_staticEntityCount)
		{
			m_staticEntityCount = a_staticEntityCount;
		}

		void mapEntities(entt::entity a_ourEntity, entt::entity a_theirEntity)
		{
			VOB_AOE_CHECK_TERMINATE_SLOW(m_staticEntityCount <= entt::to_entity(a_ourEntity), "Attempting to map entities in static entities range.");
			VOB_AOE_CHECK_TERMINATE_SLOW(m_staticEntityCount <= entt::to_entity(a_theirEntity), "Attempting to map entities in static entities range.");
			
			auto const ourOffset = static_cast<size_t>(entt::to_entity(a_theirEntity)) - static_cast<size_t>(m_staticEntityCount);
			if (ourOffset >= m_toOurs.size())
			{
				m_toOurs.resize(ourOffset + 1);
			}
			m_toOurs[ourOffset].version = entt::to_version(a_theirEntity);
			m_toOurs[ourOffset].mappedEntity = a_ourEntity;

			auto const theirOffset = static_cast<size_t>(entt::to_entity(a_ourEntity)) - static_cast<size_t>(m_staticEntityCount);
			if (theirOffset >= m_toTheirs.size())
			{
				m_toTheirs.resize(theirOffset + 1);
			}
			m_toTheirs[theirOffset].version = entt::to_version(a_ourEntity);
			m_toTheirs[theirOffset].mappedEntity = a_theirEntity;
		}

	private:
		struct EntityMapping
		{
			entt::entity mappedEntity = entt::null;
			entt::entt_traits<entt::entity>::version_type version = 0;
		};

		entt::entt_traits<entt::entity>::entity_type m_staticEntityCount = 0;
		std::vector<EntityMapping> m_toOurs;
		std::vector<EntityMapping> m_toTheirs;
	};
}
