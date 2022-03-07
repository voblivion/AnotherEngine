#pragma once

#include <typeindex>
#include <unordered_set>

#include <vob/aoe/ecs/entity_manager.h>
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
		using EntityType = entity_view<ComponentTypes...>;
	};

	// ReSharper disable once CppClassCanBeFinal
	class WorldDataProvider
	{
	public:
		// Constructors
		explicit WorldDataProvider(world_data& a_worldData)
			: m_worldData{ a_worldData }
		{}
		virtual ~WorldDataProvider() = default;

		// Methods
		stop_manager& getStopManager()
		{
			onGetStopManager();
			return m_worldData.m_stopManager;
		}

		template <typename ComponentType>
		ComponentType* getWorldComponent()
		{
			onGetWorldComponent(makeResourceAccess<ComponentType>());
			return m_worldData.m_worldComponents.get_component<ComponentType>();
		}

		template <typename ComponentType>
		ComponentType& getWorldComponentRef()
		{
			auto component = getWorldComponent<ComponentType>();
			assert(component != nullptr);
			return *component;
		}

		template <typename SystemType, typename... ComponentTypes>
		entity_view_list<ComponentTypes...> const& getentity_view_list(SystemType& a_system)
		{
			onGetEntityList({ makeResourceAccess<ComponentTypes>()... });
			return m_worldData.m_entityManager.getentity_view_list<
				SystemType, ComponentTypes...>(a_system);
		}

		template <typename SystemType, typename... ComponentTypes>
		entity_view_list<ComponentTypes...> const& getentity_view_list(
			SystemType& a_system, ComponentTypeList<ComponentTypes...> const&
		)
		{
			return getentity_view_list<SystemType, ComponentTypes...>(a_system);
		}

		spawn_manager& get_spawn_manager()
		{
			onGetSpawnManager();
			return m_worldData.m_entityManager.get_spawn_manager();
		}

		unspawn_manager& get_unspawn_manager()
		{
			onGetUnspawnManager();
			return m_worldData.m_entityManager.get_unspawn_manager();
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