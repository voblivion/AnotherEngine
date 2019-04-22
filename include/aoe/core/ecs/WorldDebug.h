#pragma once

#include <unordered_set>
#include <aoe/core/ecs/WorldData.h>
// TMP
#include <iostream>


namespace vob
{
	/*using ResourceControl = std::unordered_map<std::type_index, std::pair<unsigned, bool>>;

	// ReSharper disable once CppClassCanBeFinal
	class WorldDataProvider
		: public ADynamicType
	{
	public:
		// Constructors
		explicit WorldDataProvider(WorldData& a_worldData, ResourceControl& a_resourceControl)
			: m_worldData{ a_worldData }
			, m_resourceControl{ a_resourceControl }
		{}

		// Methods
		bool& getStopBool()
		{
			onGetStopBool();
			return m_worldData.m_shouldStop;
		}

		template <typename ComponentType>
		ComponentType* getWorldComponent()
		{
			onGetWorldComponent(access<ComponentType>());
			return m_worldData.m_worldComponents.getComponent<ComponentType>();
		}

		template <typename... ComponentTypes>
		SystemEntityList<ComponentTypes...> const& getEntityList()
		{
			onGetEntityList({ access<ComponentTypes>()... });
			return m_worldData.m_entityManager.getEntityList<ComponentTypes...>();
		}

		SystemSpawnManager& getSpawnManager()
		{
			onGetSpawnManager();
			return m_worldData.m_entityManager.getSystemSpawnManager();
		}

		SystemUnspawnManager& getUnspawnManager()
		{
			onGetUnspawnManager();
			return m_worldData.m_entityManager.getSystemUnspawnManager();
		}

		void lockResources()
		{
			for (auto const& t_pair : m_resourceAccesses)
			{
				auto const t_it = m_resourceControl.find(t_pair.first);
				if (t_pair.second)
				{
					// Writing
					assert(t_it->second.first == 0 && !t_it->second.second);
					t_it->second.second = true;
				}
				else
				{
					// Reading
					assert(!t_it->second.second);
					++t_it->second.first;
				}
			}
		}

		void unlockResources()
		{
			for (auto const& t_pair : m_resourceAccesses)
			{
				auto const t_it = m_resourceControl.find(t_pair.first);
				if (t_pair.second)
				{
					// Writing
					t_it->second.second = false;
				}
				else
				{
					// Reading
					--t_it->second.first;
				}
			}
		}

	protected:
		// Methods
		virtual void onGetStopBool() {}

		virtual void onGetWorldComponent(std::pair<std::type_index, bool> a_pair)
		{
			m_resourceControl.emplace(a_pair.first, std::make_pair(0, false));
			m_resourceAccesses.emplace(a_pair);
		}

		virtual void onGetEntityList(
			std::pmr::unordered_set<std::pair<std::type_index, bool>> a_set)
		{
			for (auto const& t_pair : a_set)
			{
				m_resourceControl.emplace(t_pair.first, std::make_pair(0, false));
			}
			m_resourceAccesses.merge(a_set);
		}

		virtual void onGetSpawnManager() {}

		virtual void onGetUnspawnManager() {}

	private:
		// Attributes
		WorldData& m_worldData;
		ResourceControl& m_resourceControl;
		std::pmr::unordered_set<std::pair<std::type_index, bool>> m_resourceAccesses;
	};*/
}
