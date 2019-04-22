#pragma once

#include <deque>
#include <unordered_map>

#include <aoe/Config.h>
#include <aoe/core/Entity.h>
#include <aoe/core/EntityId.h>
#include <aoe/core/SystemEntity.h>
#include <aoe/core/SystemSpawnManager.h>
#include <aoe/core/SystemUnspawnManager.h>
#include <aoe/standard/Memory.h>

namespace aoe
{
	namespace core
	{
		using EntityMap = std::pmr::unordered_map<EntityId, sta::PolymorphicPtr<Entity>>;

		template <typename... ComponentTypes>
		using SystemEntityList = std::pmr::deque<SystemEntity<ComponentTypes...>>;

		namespace detail
		{
			template <typename... ComponentTypes>
			struct SystemEntityTester;

			template <>
			struct SystemEntityTester<>
			{
				static bool test(Entity const& a_entity)
				{
					return true;
				}
			};

			template <typename ComponentType, typename... ComponentTypes>
			struct SystemEntityTester<ComponentType, ComponentTypes...>
			{
				static bool test(Entity const& a_entity)
				{
					return a_entity.hasComponent<std::remove_const_t<ComponentType>>()
						&& SystemEntityTester<ComponentTypes...>::test(a_entity);
				}
			};

			template <typename ComponentType, typename... ComponentTypes>
			struct SystemEntityTester<ComponentType*, ComponentTypes...>
			{
				static bool test(Entity const& a_entity)
				{
					return SystemEntityTester<ComponentTypes...>::test(a_entity);
				}
			};

			struct AListHolder
				: public sta::ADynamicType
			{
				// Methods
				virtual void onEntityAdded(Entity const& a_entity) = 0;
				virtual void onEntityRemoved(EntityId const a_id) = 0;
			};

			template <typename... ComponentTypes>
			struct ListHolder final
				: public AListHolder
			{
				// Attributes
				SystemEntityList<ComponentTypes...> m_list;

				// Constructor
				explicit ListHolder(EntityMap const& a_entities)
					: m_list{ a_entities.get_allocator() }
					, m_entityIndexes{ a_entities.get_allocator() }
				{
					for (auto& t_pair : a_entities)
					{
						ListHolder::onEntityAdded(*t_pair.second);
					}
				}

				// Methods
				virtual void onEntityAdded(Entity const& a_entity) override
				{
					if (SystemEntityTester<ComponentTypes...>::test(a_entity))
					{
						m_entityIndexes.emplace(a_entity.getId(), m_list.size());
						m_list.emplace_back(a_entity);
					}
				}

				virtual void onEntityRemoved(EntityId const a_id) override
				{
					auto const t_it = m_entityIndexes.find(a_id);
					if (t_it != m_entityIndexes.end())
					{
						m_entityIndexes.erase(t_it);
					}
				}

			private:
				// Attributes
				std::pmr::unordered_map<EntityId, std::size_t> m_entityIndexes;
			};
		}

		class EntityManager
		{
		public:
			// Constructors
			AOE_API explicit EntityManager(
				sta::Allocator<std::byte> const& a_allocator);

			//Methods
			AOE_API SystemSpawnManager& getSystemSpawnManager();

			AOE_API SystemUnspawnManager& getSystemUnspawnManager();

			AOE_API void processSpawns();

			AOE_API void processUnspawns();

			template <typename... ComponentTypes>
			SystemEntityList<ComponentTypes...> const& getEntityList()
			{
				auto t_listHolder = sta::allocatePolymorphic<detail::ListHolder<
					ComponentTypes...>>(m_systemEntityLists.get_allocator()
						, m_entities);
				auto& t_list = t_listHolder->m_list;
				m_systemEntityLists.emplace_back(std::move(t_listHolder));
				return t_list;
			}

		private:
			// Attributes
			EntityMap m_entities;
			std::pmr::deque<sta::PolymorphicPtr<
				detail::AListHolder>> m_systemEntityLists;

			std::pmr::vector<sta::PolymorphicPtr<Entity>> m_frameSpawns;
			SystemSpawnManager m_systemSpawnManager;

			std::pmr::vector<EntityId> m_frameUnspawns;
			SystemUnspawnManager m_systemUnspawnManager;
		};
	}
}
