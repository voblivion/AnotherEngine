#pragma once

#include <typeindex>
#include <unordered_set>

#include <vob/aoe/ecs/_entity_manager.h>
#include <vob/aoe/ecs/world_data.h>
#include <vob/aoe/core/type/ADynamicType.h>
// TMP
#include <iostream>

namespace vob::aoecs
{
	template <typename... ComponentTypes>
	struct ComponentTypeList
	{
		using EntityType = _aoecs::entity_view<ComponentTypes...>;
	};

	// ReSharper disable once CppClassCanBeFinal
	class world_data_provider
	{
	public:
		// Constructors
		explicit world_data_provider(aoecs::world_data& a_worldData)
			: m_worldData{ a_worldData }
		{}
		virtual ~world_data_provider() = default;

		// Methods
		aoecs::stop_manager& get_stop_manager()
		{
			return m_worldData.m_stopManager;
		}

		template <typename ComponentType>
		ComponentType& get_world_component()
		{
			auto const component = m_worldData.m_worldComponents.find<ComponentType>();
			assert(component != nullptr);
			return *component;
		}

		entity_manager::spawner& get_spawner()
		{
			return m_worldData.m_entityManager.get_spawner();
		}

		entity_manager::despawner& get_despawner()
		{
			return m_worldData.m_entityManager.get_despawner();
		}

		template <typename... TComponents>
		decltype(auto) observe_entities()
		{
			return m_worldData.m_entityManager.observe_entities<TComponents...>();
		}

		template <typename TSystem>
		void observe_spawns(TSystem const& a_system)
		{
			m_worldData.m_entityManager.observe_spawns(a_system);
		}

		template <typename TSystem>
		void observe_despawns(TSystem const& a_system)
		{
			m_worldData.m_entityManager.observe_despawns(a_system);
		}

	private:
		// Attributes
		world_data& m_worldData;
	};
}