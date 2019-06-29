#pragma once

#include <deque>
#include <unordered_map>

#include <aoe/core/Export.h>
#include <aoe/core/ecs/Entity.h>
#include <aoe/core/ecs/EntityId.h>
#include <aoe/core/ecs/SystemEntity.h>
#include <aoe/core/ecs/SystemSpawnManager.h>
#include <aoe/core/ecs/SystemUnspawnManager.h>
#include <aoe/core/standard/Memory.h>

namespace aoe
{
	namespace ecs
	{
		using EntityMap = std::pmr::unordered_map<EntityId, sta::PolymorphicPtr<Entity>>;

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
		}

		struct ASystemEntityList
			: public sta::ADynamicType
		{
			// Methods
			virtual void onEntityAdded(Entity const& a_entity) = 0;
			virtual void onEntityRemoved(EntityId const a_id) = 0;
		};

		template <typename... ComponentTypes>
		struct SystemEntityList final
			: public ASystemEntityList
		{
			// Attributes
			std::pmr::deque<SystemEntity<ComponentTypes...>> m_list;
			std::pmr::unordered_map<EntityId, std::size_t> m_entityIndexes;

			// Constructor
			explicit SystemEntityList(EntityMap const& a_entities)
				: m_list{ a_entities.get_allocator() }
				, m_entityIndexes{ a_entities.get_allocator() }
			{
				for (auto& t_pair : a_entities)
				{
					SystemEntityList::onEntityAdded(*t_pair.second);
				}
			}

			// Methods
			virtual void onEntityAdded(Entity const& a_entity) override
			{
				if (detail::SystemEntityTester<ComponentTypes...>::test(a_entity))
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

			bool empty() const
			{
				return m_list.empty();
			}

			auto begin()
			{
				return m_list.begin();
			}

			auto begin() const
			{
				return m_list.begin();
			}

			auto const& front() const
			{
				return *begin();
			}

			auto end()
			{
				return m_list.end();
			}

			auto end() const
			{
				return m_list.end();
			}

			SystemEntity<ComponentTypes...> const* find(
				EntityId const a_id) const
			{
				auto const t_it = m_entityIndexes.find(a_id);
				if (t_it != m_entityIndexes.end())
				{
					return &m_list[t_it->second];
				}
				return nullptr;
			}
		};

		class EntityManager
		{
		public:
			// Constructors
			AOE_CORE_API explicit EntityManager(
				sta::Allocator<std::byte> const& a_allocator);

			//Methods
			AOE_CORE_API SystemSpawnManager& getSystemSpawnManager();

			AOE_CORE_API SystemUnspawnManager& getSystemUnspawnManager();

			AOE_CORE_API void update();

			template <typename... ComponentTypes>
			SystemEntityList<ComponentTypes...> const& getEntityList()
			{
				auto t_listHolder = sta::allocatePolymorphic<SystemEntityList<
					ComponentTypes...>>(m_systemEntityLists.get_allocator()
						, m_entities);
				auto& t_list = *t_listHolder;
				m_systemEntityLists.emplace_back(std::move(t_listHolder));
				return t_list;
			}

		private:
			// Attributes
			EntityMap m_entities;
			std::pmr::deque<sta::PolymorphicPtr<
				ASystemEntityList>> m_systemEntityLists;

			std::pmr::vector<sta::PolymorphicPtr<Entity>> m_frameSpawns;
			SystemSpawnManager m_systemSpawnManager;

			std::pmr::vector<EntityId> m_frameUnspawns;
			SystemUnspawnManager m_systemUnspawnManager;

			// Methods
			void processSpawns();

			void processUnspawns();
		};
	}
}
