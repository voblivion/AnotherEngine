#pragma once

#include <typeindex>
#include <unordered_set>

#include <vob/aoe/core/ecs/EntityManager.h>
#include <vob/aoe/core/ecs/WorldData.h>
#include <vob/aoe/core/type/ADynamicType.h>
// TMP
#include <iostream>

namespace vob::aoe::ecs
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
	struct hash<vob::aoe::ecs::ResourceAccess>
	{
		std::size_t operator()(
			vob::aoe::ecs::ResourceAccess const& a_resourceAccess
		) const noexcept
		{
			return hash<std::type_index>{}(a_resourceAccess.m_resourceTypeIndex)
				^ ~(hash<vob::aoe::ecs::ResourceAccess::AccessType>{}(
					a_resourceAccess.m_accessType)
				);
		}
	};
}

namespace vob::aoe::ecs
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
		using EntityType = SystemEntity<ComponentTypes...>;
	};

	// ReSharper disable once CppClassCanBeFinal
	class WorldDataProvider
	{
	public:
		// Constructors
		explicit WorldDataProvider(WorldData& a_worldData)
			: m_worldData{ a_worldData }
		{}
		virtual ~WorldDataProvider() = default;

		// Methods
		bool& getStopBool()
		{
			onGetStopBool();
			return m_worldData.m_shouldStop;
		}

		template <typename ComponentType>
		ComponentType* getWorldComponent()
		{
			onGetWorldComponent(makeResourceAccess<ComponentType>());
			return m_worldData.m_worldComponents.getComponent<ComponentType>();
		}

		template <typename ComponentType>
		ComponentType& getWorldComponentRef()
		{
			auto component = getWorldComponent<ComponentType>();
			assert(component != nullptr);
			return *component;
		}

		template <typename SystemType, typename... ComponentTypes>
		EntityList<ComponentTypes...> const& getEntityList(SystemType& a_system)
		{
			onGetEntityList({ makeResourceAccess<ComponentTypes>()... });
			return m_worldData.m_entityManager.getEntityList<
				SystemType, ComponentTypes...>(a_system);
		}

		template <typename SystemType, typename... ComponentTypes>
		EntityList<ComponentTypes...> const& getEntityList(
			SystemType& a_system, ComponentTypeList<ComponentTypes...> const&
		)
		{
			return getEntityList<SystemType, ComponentTypes...>(a_system);
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

	protected:
		// Methods
		virtual void onGetStopBool() {}

		virtual void onGetWorldComponent(ResourceAccess) {}

		virtual void onGetEntityList(std::unordered_set<ResourceAccess>) {}

		virtual void onGetSpawnManager() {}

		virtual void onGetUnspawnManager() {}

	private:
		// Attributes
		WorldData& m_worldData;
	};
}