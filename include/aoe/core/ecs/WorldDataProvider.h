#pragma once

#include <typeindex>

#include <unordered_set>
#include <aoe/core/ecs/EntityManager.h>
#include <aoe/core/ecs/WorldData.h>
#include <aoe/core/standard/ADynamicType.h>
// TMP
#include <iostream>

namespace aoe
{
	namespace ecs
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
}

namespace std
{
	template <>
	struct hash<aoe::ecs::ResourceAccess>
	{
		std::size_t operator()(
			aoe::ecs::ResourceAccess const& a_resourceAccess) const noexcept
		{
			return hash<std::type_index>{}(a_resourceAccess.m_resourceTypeIndex)
				^ ~(hash<aoe::ecs::ResourceAccess::AccessType>{}(
					a_resourceAccess.m_accessType));
		}
	};
}

namespace aoe
{
	namespace ecs
	{
		template <typename Type>
		ResourceAccess makeResourceAccess()
		{
			return { typeid(std::remove_const_t<Type>), std::is_const_v<Type>
				? ResourceAccess::AccessType::Read
				: ResourceAccess::AccessType::Write };
		}

		template <typename... ComponentTypes>
		struct ComponentTypeList {};

		// ReSharper disable once CppClassCanBeFinal
		class WorldDataProvider
			: public sta::ADynamicType
		{
		public:
			// Constructors
			explicit WorldDataProvider(WorldData& a_worldData)
				: m_worldData{ a_worldData }
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
				onGetWorldComponent(makeResourceAccess<ComponentType>());
				return m_worldData.m_worldComponents.getComponent<ComponentType>();
			}

			template <typename... ComponentTypes>
			SystemEntityList<ComponentTypes...> const& getEntityList()
			{
				onGetEntityList({ makeResourceAccess<ComponentTypes>()... });
				return m_worldData.m_entityManager.getEntityList<ComponentTypes...>();
			}

			template <typename... ComponentTypes>
			SystemEntityList<ComponentTypes...> const& getEntityList(
				ComponentTypeList<ComponentTypes...> const&)
			{
				return getEntityList<ComponentTypes...>();
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

			virtual void onGetEntityList(
				std::pmr::unordered_set<ResourceAccess>) {} // NOLINT

			virtual void onGetSpawnManager() {}

			virtual void onGetUnspawnManager() {}

		private:
			// Attributes
			WorldData& m_worldData;
		};
	}
}