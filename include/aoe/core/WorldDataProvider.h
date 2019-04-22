#pragma once

#include <typeindex>

#include <unordered_set>
#include <aoe/core/EntityManager.h>
#include <aoe/core/WorldData.h>
#include <aoe/standard/ADynamicType.h>
// TMP
#include <iostream>

namespace aoe
{
	namespace core
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
	struct hash<aoe::core::ResourceAccess>
	{
		std::size_t operator()(
			aoe::core::ResourceAccess const& a_resourceAccess) const noexcept
		{
			return hash<std::type_index>{}(a_resourceAccess.m_resourceTypeIndex)
				^ ~(hash<aoe::core::ResourceAccess::AccessType>{}(
					a_resourceAccess.m_accessType));
		}
	};
}

namespace aoe
{
	namespace core
	{
		template <typename Type>
		ResourceAccess makeResourceAccess()
		{
			return { typeid(std::remove_const_t<Type>), std::is_const_v<Type>
				? ResourceAccess::AccessType::Read
				: ResourceAccess::AccessType::Write };
		}

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