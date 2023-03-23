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
	struct ResourceAccess
	{
		enum class AccessType
		{
			Read,
			Write
		};

		std::type_index m_resourceTypeIndex;
		AccessType m_accessType;

		friend bool operator==(ResourceAccess const& a_lhs
			, ResourceAccess const& a_rhs)
		{
			return a_lhs.m_resourceTypeIndex == a_rhs.m_resourceTypeIndex
				&& a_lhs.m_accessType == a_rhs.m_accessType;
		}
	};
}

namespace std
{
	template <>
	struct hash<vob::aoecs::ResourceAccess>
	{
		std::size_t operator()(
			vob::aoecs::ResourceAccess const& a_resourceAccess
		) const noexcept
		{
			return hash<std::type_index>{}(a_resourceAccess.m_resourceTypeIndex)
				^ ~(hash<vob::aoecs::ResourceAccess::AccessType>{}(
					a_resourceAccess.m_accessType)
				);
		}
	};
}

namespace vob::aoecs
{
	template <typename Type>
	ResourceAccess makeResourceAccess()
	{
		return { typeid(std::remove_const_t<Type>), std::is_const_v<Type>
			? ResourceAccess::AccessType::Read
			: ResourceAccess::AccessType::Write };
	}

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
		aoecs::stop_manager& getStopManager()
		{
			onGetStopManager();
			return m_worldData.m_stopManager;
		}

		aoecs::stop_manager& get_stop_manager()
		{
			onGetStopManager();
			return m_worldData.m_stopManager;
		}

		template <typename ComponentType>
		ComponentType& get_world_component()
		{
			onGetWorldComponent(makeResourceAccess<ComponentType>());
			auto const component = m_worldData.m_worldComponents.find<ComponentType>();
			assert(component != nullptr);
			return *component;
		}

		template <typename SystemType, typename... ComponentTypes>
		_aoecs::entity_view_list<ComponentTypes...> const& get_old_entity_view_list(SystemType& a_system)
		{
			onGetEntityList({ makeResourceAccess<ComponentTypes>()... });
			return m_worldData.m_oldEntityManager.get_entity_view_list<
				SystemType, ComponentTypes...>(a_system);
		}

		template <typename SystemType, typename... ComponentTypes>
		_aoecs::entity_view_list<ComponentTypes...> const& get_old_entity_view_list(
			SystemType& a_system, ComponentTypeList<ComponentTypes...> const&
		)
		{
			return get_old_entity_view_list<SystemType, ComponentTypes...>(a_system);
		}

		template <typename TSystem, typename... TComponents>
		_aoecs::entity_view_list<TComponents...> const& get_old_entity_view_list(
			TSystem& a_system, _aoecs::entity_view_list<TComponents...>& a_list)
		{
			return get_old_entity_view_list<TSystem, TComponents...>(a_system);
		}

		_aoecs::spawn_manager& get_old_spawn_manager()
		{
			onGetSpawnManager();
			return m_worldData.m_oldEntityManager.get_spawn_manager();
		}

		_aoecs::unspawn_manager& get_old_unspawn_manager()
		{
			onGetUnspawnManager();
			return m_worldData.m_oldEntityManager.get_unspawn_manager();
		}

		entity_manager::spawner& get_spawner()
		{
			onGetSpawnManager();
			return m_worldData.m_entityManager.get_spawner();
		}

		entity_manager::despawner& get_despawner()
		{
			onGetSpawnManager();
			return m_worldData.m_entityManager.get_despawner();
		}

		template <typename... TComponents>
		decltype(auto) observe_entities()
		{
			onGetEntityList({ makeResourceAccess<TComponents>()... });
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

	protected:
		// Methods
		virtual void onGetStopManager() {}

		virtual void onGetWorldComponent(ResourceAccess) {}

		virtual void onGetEntityList(std::unordered_set<ResourceAccess>) {}

		virtual void onGetSpawnManager() {}

		virtual void onGetUnspawnManager() {}

	private:
		// Attributes
		world_data& m_worldData;
	};
}